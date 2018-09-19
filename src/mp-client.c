/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <ctype.h>

#include "mp-client.h"
#include "mp-instance.h"

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

gchar **
split_line (const gchar *line, int max_split)
{
    const gchar *c = line;

    g_autoptr(GPtrArray) tokens = g_ptr_array_new ();
    while (*c != '\0') {
        const gchar *start = c;
        while (*c != '\0' && !isspace (*c))
            c++;

        if (max_split > 0 && tokens->len >= max_split - 1)
            while (*c != '\0')
                c++;

        if (c != start)
            g_ptr_array_add (tokens, g_strndup (start, c - start));

        while (isspace (*c))
            c++;
    }
    g_ptr_array_add (tokens, NULL);

    return (gchar **) g_steal_pointer (&tokens->pdata);
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

gchar *
mp_client_get_version_sync (MpClient *client, GCancellable *cancellable, GError **error)
{
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, error, "multipass", "version", NULL);
    if (subprocess == NULL)
        return NULL;
    g_autofree gchar *output = NULL;
    if (!g_subprocess_communicate_utf8 (subprocess, NULL, cancellable, &output, NULL, error))
        return NULL;

    g_auto(GStrv) lines = g_strsplit (output, "\n", -1);
    for (int i = 0; lines[i] != NULL; i++) {
        g_auto(GStrv) tokens = split_line (lines[i], -1);
        if (g_strv_length (tokens) < 2)
            continue;

        if (g_strcmp0 (tokens[0], "multipassd") == 0)
            return g_strdup (tokens[1]);
    }

    // FIXME: Generate an error
    return NULL;
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
    g_autoptr(GPtrArray) instances = g_ptr_array_new_with_free_func (g_object_unref);
    for (int i = 0; lines[i] != NULL; i++) {
        /* Skip header line */
        if (i == 0)
            continue;

        g_auto(GStrv) tokens = split_line (lines[i], 4);
        if (tokens[0] == NULL)
            continue;

        g_autoptr(MpInstance) instance = g_object_new (MP_TYPE_INSTANCE,
                                                       "name", tokens[0],
                                                       "state", tokens[1],
                                                       "ipv4", tokens[2],
                                                       "release", tokens[3],
                                                       NULL);

        g_ptr_array_add (instances, g_object_ref (instance));
    }

    g_task_return_pointer (task, g_steal_pointer (&instances), (GDestroyNotify) g_ptr_array_unref);
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

        g_auto(GStrv) tokens = split_line (lines[i], -1);
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

static void
launch_cb (GObject *object, GAsyncResult *result, gpointer user_data)
{
    GSubprocess *subprocess = G_SUBPROCESS (object);
    g_autoptr(GTask) task = user_data;

    g_autofree gchar *output = NULL;
    g_autoptr(GError) error = NULL;
    if (!g_subprocess_communicate_utf8_finish (subprocess, result, &output, NULL, &error)) {
        g_task_return_error (task, g_steal_pointer (&error));
        return;
    }

    g_task_return_boolean (task, TRUE);
}

void
mp_client_launch_async (MpClient *client, const gchar *name, const gchar *image, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_autoptr(GTask) task = g_task_new (client, cancellable, callback, callback_data);

    g_autoptr(GError) error = NULL;
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, "multipass", "launch", "--name", name, image, NULL);
    if (subprocess == NULL) {
        g_warning ("Failed to make subprocess: %s\n", error->message);
        return;
    }
    g_subprocess_communicate_utf8_async (subprocess, NULL, cancellable, launch_cb, g_steal_pointer (&task));
}

gboolean
mp_client_launch_finish (MpClient *client, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (G_TASK (result), client), FALSE);
    return g_task_propagate_boolean (G_TASK (result), error);
}

static void
start_cb (GObject *object, GAsyncResult *result, gpointer user_data)
{
    GSubprocess *subprocess = G_SUBPROCESS (object);
    g_autoptr(GTask) task = user_data;

    g_autofree gchar *output = NULL;
    g_autoptr(GError) error = NULL;
    if (!g_subprocess_communicate_utf8_finish (subprocess, result, &output, NULL, &error)) {
        g_task_return_error (task, g_steal_pointer (&error));
        return;
    }

    g_task_return_boolean (task, TRUE);
}

void
mp_client_start_async (MpClient *client, const gchar *name, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_autoptr(GTask) task = g_task_new (client, cancellable, callback, callback_data);

    g_autoptr(GError) error = NULL;
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, "multipass", "start", name, NULL);
    if (subprocess == NULL) {
        g_warning ("Failed to make subprocess: %s\n", error->message);
        return;
    }
    g_subprocess_communicate_utf8_async (subprocess, NULL, cancellable, start_cb, g_steal_pointer (&task));
}

gboolean
mp_client_start_finish (MpClient *client, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (G_TASK (result), client), FALSE);
    return g_task_propagate_boolean (G_TASK (result), error);
}

static void
stop_cb (GObject *object, GAsyncResult *result, gpointer user_data)
{
    GSubprocess *subprocess = G_SUBPROCESS (object);
    g_autoptr(GTask) task = user_data;

    g_autofree gchar *output = NULL;
    g_autoptr(GError) error = NULL;
    if (!g_subprocess_communicate_utf8_finish (subprocess, result, &output, NULL, &error)) {
        g_task_return_error (task, g_steal_pointer (&error));
        return;
    }

    g_task_return_boolean (task, TRUE);
}

void
mp_client_stop_async (MpClient *client, const gchar *name, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_autoptr(GTask) task = g_task_new (client, cancellable, callback, callback_data);

    g_autoptr(GError) error = NULL;
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, "multipass", "stop", name, NULL);
    if (subprocess == NULL) {
        g_warning ("Failed to make subprocess: %s\n", error->message);
        return;
    }
    g_subprocess_communicate_utf8_async (subprocess, NULL, cancellable, stop_cb, g_steal_pointer (&task));
}

gboolean
mp_client_stop_finish (MpClient *client, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (G_TASK (result), client), FALSE);
    return g_task_propagate_boolean (G_TASK (result), error);
}

static void
delete_cb (GObject *object, GAsyncResult *result, gpointer user_data)
{
    GSubprocess *subprocess = G_SUBPROCESS (object);
    g_autoptr(GTask) task = user_data;

    g_autofree gchar *output = NULL;
    g_autoptr(GError) error = NULL;
    if (!g_subprocess_communicate_utf8_finish (subprocess, result, &output, NULL, &error)) {
        g_task_return_error (task, g_steal_pointer (&error));
        return;
    }

    g_task_return_boolean (task, TRUE);
}

void
mp_client_delete_async (MpClient *client, const gchar *name, gboolean purge, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_autoptr(GTask) task = g_task_new (client, cancellable, callback, callback_data);

    g_autoptr(GError) error = NULL;
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, "multipass", "delete", name, purge ? "--purge" : NULL, NULL);
    if (subprocess == NULL) {
        g_warning ("Failed to make subprocess: %s\n", error->message);
        return;
    }
    g_subprocess_communicate_utf8_async (subprocess, NULL, cancellable, delete_cb, g_steal_pointer (&task));
}

gboolean
mp_client_delete_finish (MpClient *client, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (G_TASK (result), client), FALSE);
    return g_task_propagate_boolean (G_TASK (result), error);
}

static void
recover_cb (GObject *object, GAsyncResult *result, gpointer user_data)
{
    GSubprocess *subprocess = G_SUBPROCESS (object);
    g_autoptr(GTask) task = user_data;

    g_autofree gchar *output = NULL;
    g_autoptr(GError) error = NULL;
    if (!g_subprocess_communicate_utf8_finish (subprocess, result, &output, NULL, &error)) {
        g_task_return_error (task, g_steal_pointer (&error));
        return;
    }

    g_task_return_boolean (task, TRUE);
}

void
mp_client_recover_async (MpClient *client, const gchar *name, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_autoptr(GTask) task = g_task_new (client, cancellable, callback, callback_data);

    g_autoptr(GError) error = NULL;
    g_autoptr(GSubprocess) subprocess = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, "multipass", "recover", name, NULL);
    if (subprocess == NULL) {
        g_warning ("Failed to make subprocess: %s\n", error->message);
        return;
    }
    g_subprocess_communicate_utf8_async (subprocess, NULL, cancellable, recover_cb, g_steal_pointer (&task));
}

gboolean
mp_client_recover_finish (MpClient *client, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (G_TASK (result), client), FALSE);
    return g_task_propagate_boolean (G_TASK (result), error);
}
