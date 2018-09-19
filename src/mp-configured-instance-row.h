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

#include <gtk/gtk.h>

#include "mp-instance.h"

G_BEGIN_DECLS

#define MP_TYPE_CONFIGURED_INSTANCE_ROW (mp_configured_instance_row_get_type ())
G_DECLARE_FINAL_TYPE (MpConfiguredInstanceRow, mp_configured_instance_row, MP, CONFIGURED_INSTANCE_ROW, GtkListBoxRow)

MpConfiguredInstanceRow *mp_configured_instance_row_new          (void);

void                     mp_configured_instance_row_set_instance (MpConfiguredInstanceRow *row,
                                                                  MpInstance              *instance);

MpInstance              *mp_configured_instance_row_get_instance (MpConfiguredInstanceRow *row);

G_END_DECLS
