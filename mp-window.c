#include "mp-client.h"
#include "mp-instance-row.h"
#include "mp-launch-dialog.h"
#include "mp-window.h"

struct _MpWindow
{
    GtkWindow      parent_instance;

    GtkListBox    *instances_listbox;
    GtkListBoxRow *add_row;

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
}

static void
instance_row_activated_cb (MpWindow *window, GtkListBoxRow *row)
{
    if (row == window->add_row) {
        MpLaunchDialog *dialog = mp_launch_dialog_new (window->client);
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (GTK_WIDGET (dialog));
    }
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
    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_listbox);

    gtk_widget_class_bind_template_callback (widget_class, instance_row_activated_cb);
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

    window->cancellable = g_cancellable_new ();
    window->client = mp_client_new ();
    mp_client_list_async (window->client, window->cancellable, list_cb, window);
}

MpWindow *
mp_window_new (void)
{
    return g_object_new (MP_TYPE_WINDOW, NULL);
}
