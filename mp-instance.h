#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define MP_TYPE_INSTANCE (mp_instance_get_type ())
G_DECLARE_FINAL_TYPE (MpInstance, mp_instance, MP, INSTANCE, GObject)

const gchar *mp_instance_get_name  (MpInstance *instance);

const gchar *mp_instance_get_state (MpInstance *instance);

G_END_DECLS
