#include <vte/vte.h>

#include "mp-client.h"
#include "mp-instance-row.h"
#include "mp-launch-dialog.h"
#include "mp-window.h"

struct _MpWindow
{
    GtkWindow      parent_instance;

    GtkListBoxRow *add_row;
    GtkBox        *image_box;
    GtkBox        *instances_box;
    GtkListBox    *instances_listbox;
    GtkStack      *instances_stack;
    GtkStack      *main_stack;
    VteTerminal   *terminal;

    GCancellable  *cancellable;
    MpClient      *client;
};

G_DEFINE_TYPE (MpWindow, mp_window, GTK_TYPE_WINDOW)

static void
add_row (MpWindow *window, const gchar *name)
{
    MpInstanceRow *row = mp_instance_row_new (name);
    gtk_widget_show (GTK_WIDGET (row));
    gtk_list_box_insert (window->instances_listbox, GTK_WIDGET (row), gtk_list_box_row_get_index (window->add_row));

    gtk_stack_set_visible_child (window->instances_stack, GTK_WIDGET (window->image_box));

    if (gtk_list_box_get_selected_row (window->instances_listbox) == NULL)
       gtk_list_box_select_row (window->instances_listbox, GTK_LIST_BOX_ROW (row));
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
    gtk_widget_class_bind_template_child (widget_class, MpWindow, image_box);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_box);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_listbox);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_stack);
    gtk_widget_class_bind_template_child (widget_class, MpWindow, main_stack);

    gtk_widget_class_bind_template_callback (widget_class, instance_row_activated_cb);
    gtk_widget_class_bind_template_callback (widget_class, instance_row_selected_cb);
}

static void
list_cb (GObject *client, GAsyncResult *result, gpointer user_data)
{
    MpWindow *window = user_data;

    g_autoptr(GError) error = NULL;
    g_auto(GStrv) instance_names = mp_client_list_finish (window->client, result, &error);
    if (instance_names == NULL) {
        g_printerr ("Failed to get instances: %s\n", error->message);
        return;
    }

    for (int i = 0; instance_names[i] != NULL; i++) {
        add_row (window, instance_names[i]);
    }
}

void
mp_window_init (MpWindow *window)
{
    gtk_widget_init_template (GTK_WIDGET (window));

    window->terminal = VTE_TERMINAL (vte_terminal_new ());
    gtk_widget_show (GTK_WIDGET (window->terminal));
    gtk_container_add (GTK_CONTAINER (window->image_box), GTK_WIDGET (window->terminal));

    window->cancellable = g_cancellable_new ();
    window->client = mp_client_new ();

    g_autoptr(GError) error = NULL;
    g_autofree gchar *version = mp_client_get_version_sync (window->client, window->cancellable, &error);

    if (version != NULL)
        gtk_stack_set_visible_child (window->main_stack, GTK_WIDGET (window->instances_box));

    mp_client_list_async (window->client, window->cancellable, list_cb, window);
}

MpWindow *
mp_window_new (void)
{
    return g_object_new (MP_TYPE_WINDOW, NULL);
}
