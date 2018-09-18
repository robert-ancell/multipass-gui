#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define MP_TYPE_CLIENT (mp_client_get_type ())
G_DECLARE_FINAL_TYPE (MpClient, mp_client, MP, CLIENT, GObject)

MpClient  *mp_client_new              (void);

gchar     *mp_client_generate_name    (MpClient            *client);

gchar     *mp_client_get_version_sync (MpClient            *client,
                                       GCancellable        *cancellable,
                                       GError             **error);

void       mp_client_list_async       (MpClient            *client,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             callback_data);

GPtrArray *mp_client_list_finish      (MpClient            *client,
                                       GAsyncResult        *result,
                                       GError             **error);

void       mp_client_find_async       (MpClient            *client,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             callback_data);

gchar    **mp_client_find_finish      (MpClient            *client,
                                       GAsyncResult        *result,
                                       GError             **error);

void       mp_client_launch_async     (MpClient            *client,
                                       const gchar         *name,
                                       const gchar         *image,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             callback_data);

gboolean   mp_client_launch_finish    (MpClient            *client,
                                       GAsyncResult        *result,
                                       GError             **error);

void       mp_client_start_async      (MpClient            *client,
                                       const gchar         *name,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             callback_data);

gboolean   mp_client_start_finish     (MpClient            *client,
                                       GAsyncResult        *result,
                                       GError             **error);

void       mp_client_stop_async       (MpClient            *client,
                                       const gchar         *name,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             callback_data);

gboolean   mp_client_stop_finish      (MpClient            *client,
                                       GAsyncResult        *result,
                                       GError             **error);

void       mp_client_delete_async     (MpClient            *client,
                                       const gchar         *name,
                                       gboolean             purge,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             callback_data);

gboolean   mp_client_delete_finish      (MpClient            *client,
                                       GAsyncResult        *result,
                                       GError             **error);

G_END_DECLS
