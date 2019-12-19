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
#include "mp-configured-instance-row.h"
#include "mp-details-dialog.h"
#include "mp-instance.h"
#include "mp-launch-dialog.h"
#include "mp-running-instance-row.h"
#include "mp-window.h"

struct _MpWindow
{
    GtkWindow      parent_instance;

    GtkLabel      *configured_instances_label;
    GtkListBox    *configured_instances_listbox;
    GtkStack      *configured_instances_stack;
    GtkButton     *shell_button;
    GtkButton     *create_button;
    GtkButton     *details_button;
    GtkBox        *instances_box;
    GtkStack      *main_stack;
    GtkLabel      *running_instances_label;
    GtkListBox    *running_instances_listbox;
    GtkStack      *running_instances_stack;
    GtkButton     *start_button;
    GtkButton     *stop_button;
    GtkButton     *trash_button;

    GCancellable  *cancellable;
    MpClient      *client;
    GSource       *list_timeout_source;
};

G_DEFINE_TYPE (MpWindow, mp_window, GTK_TYPE_WINDOW)

static MpConfiguredInstanceRow *
find_configured_row (MpWindow *window, const gchar *name)
{
    g_autoptr(GList) children = gtk_container_get_children (GTK_CONTAINER (window->configured_instances_listbox));
    for (GList *link = children; link; link = link->next) {
        if (!MP_IS_CONFIGURED_INSTANCE_ROW (link->data))
            continue;

        MpConfiguredInstanceRow *row = link->data;
        if (g_strcmp0 (mp_instance_get_name (mp_configured_instance_row_get_instance (row)), name) == 0)
            return row;
    }

    return NULL;
}

static MpRunningInstanceRow *
find_running_row (MpWindow *window, const gchar *name)
{
    g_autoptr(GList) children = gtk_container_get_children (GTK_CONTAINER (window->running_instances_listbox));
    for (GList *link = children; link; link = link->next) {
        if (!MP_IS_RUNNING_INSTANCE_ROW (link->data))
            continue;

        MpRunningInstanceRow *row = link->data;
        if (g_strcmp0 (mp_instance_get_name (mp_running_instance_row_get_instance (row)), name) == 0)
            return row;
    }

    return NULL;
}

static void
configured_selection_changed_cb (MpWindow *window)
{
    g_autoptr(GList) selected_rows = gtk_list_box_get_selected_rows (window->configured_instances_listbox);

    gtk_widget_set_sensitive (GTK_WIDGET (window->start_button), selected_rows != NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (window->trash_button), selected_rows != NULL);

    gtk_button_set_label (window->trash_button, _("Trash"));
    if (selected_rows != NULL) {
        MpConfiguredInstanceRow *row = MP_CONFIGURED_INSTANCE_ROW (gtk_list_box_get_selected_row (window->configured_instances_listbox));
        MpInstance *instance = mp_configured_instance_row_get_instance (row);
        if (g_strcmp0 (mp_instance_get_state (instance), "Deleted") == 0)
            gtk_button_set_label (window->trash_button, _("Recover"));
    }
}

static void
running_selection_changed_cb (MpWindow *window)
{
    g_autoptr(GList) selected_rows = gtk_list_box_get_selected_rows (window->running_instances_listbox);

    gtk_widget_set_sensitive (GTK_WIDGET (window->stop_button), selected_rows != NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (window->details_button), selected_rows != NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (window->shell_button), selected_rows != NULL);
}

static void
create_button_clicked_cb (MpWindow *window)
{
    MpLaunchDialog *dialog = mp_launch_dialog_new (window->client);
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
        const gchar *name = mp_launch_dialog_get_name (dialog);
        g_autofree gchar *image_name = mp_launch_dialog_get_image_name (dialog);

        mp_client_launch_async (window->client, name, image_name, window->cancellable, NULL, window);
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
trash_button_clicked_cb (MpWindow *window)
{
    MpConfiguredInstanceRow *row = MP_CONFIGURED_INSTANCE_ROW (gtk_list_box_get_selected_row (window->configured_instances_listbox));
    MpInstance *instance = mp_configured_instance_row_get_instance (row);

    if (g_strcmp0 (mp_instance_get_state (instance), "Deleted") == 0)
        mp_client_recover_async (window->client,
                                 mp_instance_get_name (instance),
                                 window->cancellable,
                                 NULL, NULL);
    else
        mp_client_delete_async (window->client,
                                mp_instance_get_name (instance),
                                FALSE,
                                window->cancellable,
                                NULL, NULL);
}

static void
start_button_clicked_cb (MpWindow *window)
{
    MpConfiguredInstanceRow *row = MP_CONFIGURED_INSTANCE_ROW (gtk_list_box_get_selected_row (window->configured_instances_listbox));

    mp_client_start_async (window->client,
                           mp_instance_get_name (mp_configured_instance_row_get_instance (row)),
                           window->cancellable,
                           NULL, NULL);
}

static void
stop_button_clicked_cb (MpWindow *window)
{
    MpRunningInstanceRow *row = MP_RUNNING_INSTANCE_ROW (gtk_list_box_get_selected_row (window->running_instances_listbox));

    mp_client_stop_async (window->client,
                          mp_instance_get_name (mp_running_instance_row_get_instance (row)),
                          window->cancellable,
                          NULL, NULL);
}

static void
details_button_clicked_cb (MpWindow *window)
{
    MpRunningInstanceRow *row = MP_RUNNING_INSTANCE_ROW (gtk_list_box_get_selected_row (window->running_instances_listbox));
    MpInstance *instance = mp_running_instance_row_get_instance (row);

    MpDetailsDialog *dialog = mp_details_dialog_new (window->client, instance);
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
spawn_cb (VteTerminal *terminal, GPid pid, GError *error, gpointer user_data)
{
    if (error)
        g_warning ("Failed to spawn shell: %s\n", error->message);
}

static void
shell_button_clicked_cb (MpWindow *window)
{
    MpRunningInstanceRow *row = MP_RUNNING_INSTANCE_ROW (gtk_list_box_get_selected_row (window->running_instances_listbox));
    MpInstance *instance = mp_running_instance_row_get_instance (row);

    GtkWindow *terminal_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
    GtkHeaderBar *header_bar = GTK_HEADER_BAR (gtk_header_bar_new ());
    gtk_header_bar_set_show_close_button (header_bar, TRUE);
    gtk_header_bar_set_title (header_bar, mp_instance_get_name (instance));
    gtk_widget_show (GTK_WIDGET (header_bar));
    gtk_window_set_titlebar (terminal_window, GTK_WIDGET (header_bar));

    VteTerminal *terminal = VTE_TERMINAL (vte_terminal_new ());
    gtk_widget_show (GTK_WIDGET (terminal));
    gtk_container_add (GTK_CONTAINER (terminal_window), GTK_WIDGET (terminal));

    g_autoptr(GPtrArray) args = g_ptr_array_new ();
    g_ptr_array_add (args, "multipass");
    g_ptr_array_add (args, "shell");
    g_ptr_array_add (args, (gpointer) mp_instance_get_name (instance));
    g_ptr_array_add (args, NULL);
    vte_terminal_spawn_async (terminal,
                              VTE_PTY_DEFAULT,
                              NULL,             /* working directory */
                              (gchar **) args->pdata,
                              NULL,             /* environment */
                              G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                              NULL, NULL, NULL, /* child setup */
                              -1,               /* timeout */
                              NULL,             /* cancellable */
                              spawn_cb, window);

    gtk_window_present (terminal_window);
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

    gtk_widget_class_bind_template_child (widget_class, MpWindow, configured_instances_label);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, configured_instances_listbox);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, configured_instances_stack);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, shell_button);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, create_button);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, details_button);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_box);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, main_stack);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, running_instances_label);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, running_instances_listbox);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, running_instances_stack);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, start_button);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, stop_button);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, trash_button);

    gtk_widget_class_bind_template_callback (widget_class, configured_selection_changed_cb);
    gtk_widget_class_bind_template_callback (widget_class, shell_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, create_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, details_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, running_selection_changed_cb);
    gtk_widget_class_bind_template_callback (widget_class, start_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, stop_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, trash_button_clicked_cb);
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
        GHashTable *configured_rows = g_hash_table_new (g_str_hash, g_str_equal);
        GHashTable *running_rows = g_hash_table_new (g_str_hash, g_str_equal);
        for (int i = 0; i < instances->len; i++) {
            MpInstance *instance = g_ptr_array_index (instances, i);
            const gchar *name = mp_instance_get_name (instance);
            const gchar *state = mp_instance_get_state (instance);

            MpConfiguredInstanceRow *configured_row = find_configured_row (window, name);
            if (configured_row == NULL) {
                configured_row = mp_configured_instance_row_new ();
                gtk_widget_show (GTK_WIDGET (configured_row));
                gtk_container_add (GTK_CONTAINER (window->configured_instances_listbox), GTK_WIDGET (configured_row));
            }
            mp_configured_instance_row_set_instance (configured_row, instance);
            g_hash_table_add (configured_rows, (gpointer) name);

            if (g_strcmp0 (state, "Starting") == 0 || g_strcmp0 (state, "Running") == 0 || g_strcmp0 (state, "Restarting") == 0) {
                MpRunningInstanceRow *running_row = find_running_row (window, name);
                if (running_row == NULL) {
                    running_row = mp_running_instance_row_new ();
                    gtk_widget_show (GTK_WIDGET (running_row));
                    gtk_container_add (GTK_CONTAINER (window->running_instances_listbox), GTK_WIDGET (running_row));
                }
                mp_running_instance_row_set_instance (running_row, instance);
                g_hash_table_add (running_rows, (gpointer) name);
            }
        }

        /* Remove any unused rows */
        g_autoptr(GList) children = gtk_container_get_children (GTK_CONTAINER (window->configured_instances_listbox));
        for (GList *link = children; link; link = link->next) {
            if (!MP_IS_CONFIGURED_INSTANCE_ROW (link->data))
                continue;

            MpConfiguredInstanceRow *row = link->data;
            if (g_hash_table_lookup (configured_rows, mp_instance_get_name (mp_configured_instance_row_get_instance (row))) == NULL)
                gtk_container_remove (GTK_CONTAINER (window->configured_instances_listbox), GTK_WIDGET (row));
        }
        g_autoptr(GList) running_children = gtk_container_get_children (GTK_CONTAINER (window->running_instances_listbox));
        for (GList *link = running_children; link; link = link->next) {
            if (!MP_IS_RUNNING_INSTANCE_ROW (link->data))
                continue;

            MpRunningInstanceRow *row = link->data;
            if (g_hash_table_lookup (running_rows, mp_instance_get_name (mp_running_instance_row_get_instance (row))) == NULL)
                gtk_container_remove (GTK_CONTAINER (window->running_instances_listbox), GTK_WIDGET (row));
        }

        /* Show placeholders when empty rows */
        if (g_hash_table_size (configured_rows) > 0)
            gtk_stack_set_visible_child (window->configured_instances_stack, GTK_WIDGET (window->configured_instances_listbox));
        else
            gtk_stack_set_visible_child (window->configured_instances_stack, GTK_WIDGET (window->configured_instances_label));
        if (g_hash_table_size (running_rows) > 0)
            gtk_stack_set_visible_child (window->running_instances_stack, GTK_WIDGET (window->running_instances_listbox));
        else
            gtk_stack_set_visible_child (window->running_instances_stack, GTK_WIDGET (window->running_instances_label));

        configured_selection_changed_cb (window);
        running_selection_changed_cb (window);
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
