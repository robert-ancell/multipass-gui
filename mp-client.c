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

static gchar *
get_random_word (const gchar *resource)
{
    g_autofree gchar *uri = g_strdup_printf ("resource:%s", resource);
    g_autoptr(GFile) file = g_file_new_for_uri (uri);
    g_autofree gchar *contents = NULL;
    g_autoptr(GError) error = NULL;
    if (!g_file_load_contents (file, NULL, &contents, NULL, NULL, &error)) {
        g_warning ("Failed to read word list %s: %s\n", resource, error->message);
        return NULL;
    }

    g_auto(GStrv) words = g_strsplit (contents, "\n", -1);
    int i = g_random_int_range (0, g_strv_length (words));

    return g_strdup (words[i]);
}

gchar *
mp_client_generate_name (MpClient *client)
{
    g_autofree gchar *adjective = get_random_word ("/com/ubuntu/multipass/adjectives.txt");
    g_autofree gchar *name = get_random_word ("/com/ubuntu/multipass/names.txt");
    return g_strdup_printf ("%s-%s", adjective, name);
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
    for (int i = 0; lines[i] != NULL; i++) {
        /* Skip header line */
        if (i == 0)
            continue;

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

static void
find_cb (GObject *object, GAsyncResult *result, gpointer user_data)
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
    for (int i = 0; lines[i] != NULL; i++) {
        /* First two lines are header */
        if (i < 2)
            continue;

        /* Skip alias information */
        if (lines[i][0] == ' ')
            continue;

        g_auto(GStrv) tokens = g_strsplit (lines[i], " ", -1);
        if (tokens[0] == NULL)
            continue;

        g_ptr_array_add (instance_names, g_strdup (tokens[0]));
    }
    g_ptr_array_add (instance_names, NULL);

    g_task_return_pointer (task, g_steal_pointer (&instance_names->pdata), (GDestroyNotify) g_strfreev);
}

void
mp_client_find_async (MpClient *client, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_autoptr(GTask) task = g_task_new (client, cancellable, callback, callback_data);

    g_autoptr(GError) error = NULL;
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, "multipass", "find", NULL);
    if (subprocess == NULL) {
        g_warning ("Failed to make subprocess: %s\n", error->message);
        return;
    }
    g_subprocess_communicate_utf8_async (subprocess, NULL, cancellable, find_cb, g_steal_pointer (&task));
}

gchar **
mp_client_find_finish (MpClient *client, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (G_TASK (result), client), NULL);
    return g_task_propagate_pointer (G_TASK (result), error);
}
