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

    g_auto(GStrv) lines = g_strsplit (output, "\n", -1);
    g_autoptr(GPtrArray) instance_names = g_ptr_array_new ();
    for (int i = 1; lines[i] != NULL; i++) {
        g_auto(GStrv) tokens = g_strsplit (lines[i], " ", -1);
        if (tokens[0] == NULL)
            continue;
        g_ptr_array_add (instance_names, g_strdup (tokens[0]));
    }
    g_ptr_array_add (instance_names, NULL);

    g_task_return_pointer (task, g_steal_pointer (&instance_names->pdata), (GDestroyNotify) g_strfreev);
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

gchar **
mp_client_list_finish (MpClient *client, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (G_TASK (result), client), NULL);
    return g_task_propagate_pointer (G_TASK (result), error);
}
