#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define MP_TYPE_CLIENT (mp_client_get_type ())
G_DECLARE_FINAL_TYPE (MpClient, mp_client, MP, CLIENT, GObject)

MpClient *mp_client_new          (void);

void mp_client_list_async        (MpClient            *client,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             callback_data);

GPtrArray *mp_client_list_finish (MpClient            *client,
                                  GError             **error);

G_END_DECLS
