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

#define MP_TYPE_WINDOW (mp_window_get_type ())
G_DECLARE_FINAL_TYPE (MpWindow, mp_window, MP, WINDOW, GtkWindow)

MpWindow *mp_window_new (void);

G_END_DECLS
