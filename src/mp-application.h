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

#define MP_TYPE_APPLICATION (mp_application_get_type ())
G_DECLARE_FINAL_TYPE (MpApplication, mp_application, MP, APPLICATION, GtkApplication)

MpApplication *mp_application_new (void);

G_END_DECLS
