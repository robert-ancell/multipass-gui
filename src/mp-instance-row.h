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

G_BEGIN_DECLS

#define MP_TYPE_INSTANCE_ROW (mp_instance_row_get_type ())
G_DECLARE_FINAL_TYPE (MpInstanceRow, mp_instance_row, MP, INSTANCE_ROW, GtkListBoxRow)

MpInstanceRow *mp_instance_row_new       (const gchar   *name);

const gchar   *mp_instance_row_get_name  (MpInstanceRow *row);


void           mp_instance_row_set_state (MpInstanceRow *row,
                                          const gchar   *state);

const gchar   *mp_instance_row_get_state (MpInstanceRow *row);

G_END_DECLS
