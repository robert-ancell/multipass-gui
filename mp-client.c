#include "mp-client.h"

struct _MpClient
{
    GObject parent_instance;
};

G_DEFINE_TYPE (MpClient, mp_client, G_TYPE_OBJECT)

void
mp_client_class_init (MpClientClass *klass)
{
}

void
mp_client_init (MpClient *client)
{
}

MpClient *
mp_client_new (void)
{
    return g_object_new (MP_TYPE_CLIENT, NULL);
}

static void
list_cb (GObject *object, GAsyncResult *result, gpointer user_data)
{
    GSubprocess *subprocess = G_SUBPROCESS (object);
    g_autoptr(GTask) task = user_data;

    g_autofree gchar *output = NULL;
    g_autoptr(GError) error = NULL;
    if (!g_subprocess_communicate_utf8_finish (subprocess, result, &output, NULL, &error)) {
        g_task_return_error (task, g_steal_pointer (&error));
        return;
    }

    g_printerr ("'%s'\n", output);

    g_task_return_pointer (task, NULL, NULL);
}

void
mp_client_list_async (MpClient *client, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_autoptr(GTask) task = g_task_new (client, cancellable, callback, callback_data);

    g_autoptr(GError) error = NULL;
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, "multipass", "list", NULL);
    if (subprocess == NULL) {
        g_warning ("Failed to make subprocess: %s\n", error->message);
        return;
    }
    g_subprocess_communicate_utf8_async (subprocess, NULL, cancellable, list_cb, g_steal_pointer (&task));
}

GPtrArray *
mp_client_list_finish (MpClient *client, GError **error)
{
    return NULL;
}
