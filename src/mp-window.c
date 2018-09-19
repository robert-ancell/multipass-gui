/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <glib/gi18n.h>
#include <vte/vte.h>

#include "mp-client.h"
#include "mp-instance.h"
#include "mp-instance-row.h"
#include "mp-launch-dialog.h"
#include "mp-window.h"

struct _MpWindow
{
    GtkWindow      parent_instance;

    GtkListBoxRow *add_row;
    GtkButton     *delete_button;
    GtkBox        *image_box;
    GtkBox        *instances_box;
    GtkListBox    *instances_listbox;
    GtkStack      *instances_stack;
    GtkStack      *main_stack;
    GtkLabel      *no_instance_label;
    GtkButton     *start_stop_button;
    VteTerminal   *terminal;

    GCancellable  *cancellable;
    MpClient      *client;
    GSource       *list_timeout_source;
};

G_DEFINE_TYPE (MpWindow, mp_window, GTK_TYPE_WINDOW)

static MpInstanceRow *
find_row (MpWindow *window, const gchar *name)
{
    g_autoptr(GList) children = gtk_container_get_children (GTK_CONTAINER (window->instances_listbox));
    for (GList *link = children; link; link = link->next) {
        if (!MP_IS_INSTANCE_ROW (link->data))
            continue;

        MpInstanceRow *row = link->data;
        if (g_strcmp0 (mp_instance_row_get_name (row), name) == 0)
            return row;
    }

    return NULL;
}

static MpInstanceRow *
get_selected_row (MpWindow *window)
{
    GtkListBoxRow *row = gtk_list_box_get_selected_row (window->instances_listbox);
    if (row == NULL || !MP_IS_INSTANCE_ROW (row))
       return NULL;

    return MP_INSTANCE_ROW (row);
}

static MpInstanceRow *
add_row (MpWindow *window, const gchar *name)
{
    MpInstanceRow *row = mp_instance_row_new (name);
    gtk_widget_show (GTK_WIDGET (row));
    gtk_list_box_insert (window->instances_listbox, GTK_WIDGET (row), gtk_list_box_row_get_index (window->add_row));

    if (gtk_list_box_get_selected_row (window->instances_listbox) == NULL)
       gtk_list_box_select_row (window->instances_listbox, GTK_LIST_BOX_ROW (row));

    return row;
}

static void
instance_row_activated_cb (MpWindow *window, GtkListBoxRow *row)
{
    if (row == window->add_row) {
        MpLaunchDialog *dialog = mp_launch_dialog_new (window->client);
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
            const gchar *name = mp_launch_dialog_get_name (dialog);
            g_autofree gchar *image_name = mp_launch_dialog_get_image_name (dialog);

            mp_client_launch_async (window->client, name, image_name, window->cancellable, NULL, window);

            add_row (window, name);
        }

        gtk_widget_destroy (GTK_WIDGET (dialog));
    }
}

static void
update_action_bar (MpWindow *window)
{
    MpInstanceRow *row = get_selected_row (window);
    if (row == NULL) {
        gtk_stack_set_visible_child (window->instances_stack, GTK_WIDGET (window->no_instance_label));
        return;
    }

    gtk_stack_set_visible_child (window->instances_stack, GTK_WIDGET (window->image_box));
    const gchar *state = mp_instance_row_get_state (row);

    if (g_strcmp0 (state, "RUNNING") == 0) {
        gtk_button_set_label (window->start_stop_button, _("Stop"));
        gtk_widget_set_sensitive (GTK_WIDGET (window->start_stop_button), TRUE);
    }
    else if (g_strcmp0 (state, "STOPPED") == 0) {
        gtk_button_set_label (window->start_stop_button, _("Start"));
        gtk_widget_set_sensitive (GTK_WIDGET (window->start_stop_button), TRUE);
    }
    else
        gtk_widget_set_sensitive (GTK_WIDGET (window->start_stop_button), FALSE);

    gtk_widget_set_sensitive (GTK_WIDGET (window->delete_button), g_strcmp0 (state, "DELETED") != 0);
}

static void
spawn_cb (VteTerminal *terminal, GPid pid, GError *error, gpointer user_data)
{
    if (error)
        g_warning ("Failed to spawn shell: %s\n", error->message);
}

static void
instance_row_selected_cb (MpWindow *window, GtkListBoxRow *row)
{
    if (!MP_IS_INSTANCE_ROW (row))
        return;
    MpInstanceRow *r = MP_INSTANCE_ROW (row);

    update_action_bar (window);

    vte_terminal_reset (window->terminal, TRUE, TRUE);

    g_autoptr(GPtrArray) args = g_ptr_array_new ();
    g_ptr_array_add (args, "multipass");
    g_ptr_array_add (args, "shell");
    g_ptr_array_add (args, (gpointer) mp_instance_row_get_name (r));
    g_ptr_array_add (args, NULL);
    vte_terminal_spawn_async (window->terminal,
                              VTE_PTY_DEFAULT,
                              NULL,             /* working directory */
                              (gchar **) args->pdata,
                              NULL,             /* environment */
                              G_SPAWN_SEARCH_PATH,
                              NULL, NULL, NULL, /* child setup */
                              -1,               /* timeout */
                              NULL,             /* cancellable */
                              spawn_cb, window);
}

static void
start_stop_button_clicked_cb (MpWindow *window)
{
    MpInstanceRow *row = get_selected_row (window);
    if (row == NULL)
        return;

    const gchar *state = mp_instance_row_get_state (row);
    if (g_strcmp0 (state, "RUNNING") == 0)
        mp_client_stop_async (window->client,
                              mp_instance_row_get_name (row),
                              window->cancellable,
                              NULL, NULL);
    else if (g_strcmp0 (state, "STOPPED") == 0)
        mp_client_start_async (window->client,
                               mp_instance_row_get_name (row),
                               window->cancellable,
                               NULL, NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (window->start_stop_button), FALSE);
}

static void
delete_button_clicked_cb (MpWindow *window)
{
    MpInstanceRow *row = get_selected_row (window);
    if (row == NULL)
        return;

    mp_client_delete_async (window->client,
                            mp_instance_row_get_name (row),
                            FALSE,
                            window->cancellable,
                            NULL, NULL);
}

static void
mp_window_dispose (GObject *object)
{
    MpWindow *window = MP_WINDOW (object);

    g_cancellable_cancel (window->cancellable);
    g_clear_object (&window->cancellable);
    g_clear_object (&window->client);

    G_OBJECT_CLASS (mp_window_parent_class)->dispose (object);
}

void
mp_window_class_init (MpWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = mp_window_dispose;

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-window.ui");

    gtk_widget_class_bind_template_child (widget_class, MpWindow, add_row);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, delete_button);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, image_box);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_box);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_listbox);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_stack);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, main_stack);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, no_instance_label);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, start_stop_button);

    gtk_widget_class_bind_template_callback (widget_class, instance_row_activated_cb);
    gtk_widget_class_bind_template_callback (widget_class, instance_row_selected_cb);
    gtk_widget_class_bind_template_callback (widget_class, start_stop_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, delete_button_clicked_cb);
}

static void update_list (MpWindow *window);

static gboolean
list_timeout_cb (gpointer user_data)
{
    MpWindow *window = user_data;
    g_clear_pointer (&window->list_timeout_source, g_source_unref);
    update_list (window);
    return G_SOURCE_REMOVE;
}

static void
list_cb (GObject *client, GAsyncResult *result, gpointer user_data)
{
    MpWindow *window = user_data;

    g_autoptr(GError) error = NULL;
    g_autoptr(GPtrArray) instances = mp_client_list_finish (MP_CLIENT (client), result, &error);

    if (instances == NULL && g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        return;

    if (instances != NULL) {
        GHashTable *updated_rows = g_hash_table_new (g_str_hash, g_str_equal);
        for (int i = 0; i < instances->len; i++) {
            MpInstance *instance = g_ptr_array_index (instances, i);
            const gchar *name = mp_instance_get_name (instance);

            MpInstanceRow *row = find_row (window, name);
            if (row == NULL)
                row = add_row (window, name);
            mp_instance_row_set_state (row, mp_instance_get_state (instance));
            g_hash_table_add (updated_rows, (gpointer) name);
        }

        /* Remove any unused rows */
        g_autoptr(GList) children = gtk_container_get_children (GTK_CONTAINER (window->instances_listbox));
        for (GList *link = children; link; link = link->next) {
            if (!MP_IS_INSTANCE_ROW (link->data))
                continue;

            MpInstanceRow *row = link->data;
            if (g_hash_table_lookup (updated_rows, mp_instance_row_get_name (row)) == NULL)
                gtk_container_remove (GTK_CONTAINER (window->instances_listbox), GTK_WIDGET (row));
        }

        update_action_bar (window);
    } else {
        g_printerr ("Failed to get instances: %s\n", error->message);
    }

    window->list_timeout_source = g_timeout_source_new_seconds (1);
    g_source_set_callback (window->list_timeout_source, list_timeout_cb, window, NULL);
    g_source_attach (window->list_timeout_source, g_main_context_default ());
}

static void
update_list (MpWindow *window)
{
    mp_client_list_async (window->client, window->cancellable, list_cb, window);
}

void
mp_window_init (MpWindow *window)
{
    gtk_widget_init_template (GTK_WIDGET (window));

    window->terminal = VTE_TERMINAL (vte_terminal_new ());
    gtk_widget_show (GTK_WIDGET (window->terminal));
    gtk_widget_set_vexpand (GTK_WIDGET (window->terminal), TRUE);
    gtk_container_add (GTK_CONTAINER (window->image_box), GTK_WIDGET (window->terminal));

    window->cancellable = g_cancellable_new ();
    window->client = mp_client_new ();

    g_autoptr(GError) error = NULL;
    g_autofree gchar *version = mp_client_get_version_sync (window->client, window->cancellable, &error);

    if (version != NULL)
        gtk_stack_set_visible_child (window->main_stack, GTK_WIDGET (window->instances_box));

    update_list (window);
}

MpWindow *
mp_window_new (void)
{
    return g_object_new (MP_TYPE_WINDOW, NULL);
}
