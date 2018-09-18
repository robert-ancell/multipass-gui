/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define MP_TYPE_INSTANCE (mp_instance_get_type ())
G_DECLARE_FINAL_TYPE (MpInstance, mp_instance, MP, INSTANCE, GObject)

const gchar *mp_instance_get_name  (MpInstance *instance);

const gchar *mp_instance_get_state (MpInstance *instance);

G_END_DECLS
