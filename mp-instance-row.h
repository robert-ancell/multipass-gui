#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MP_TYPE_INSTANCE_ROW (mp_instance_row_get_type ())
G_DECLARE_FINAL_TYPE (MpInstanceRow, mp_instance_row, MP, INSTANCE_ROW, GtkListBoxRow)

MpInstanceRow *mp_instance_row_new      (const gchar   *name);

const gchar   *mp_instance_row_get_name (MpInstanceRow *row);

G_END_DECLS
