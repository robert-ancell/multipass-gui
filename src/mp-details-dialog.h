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

#include "mp-client.h"
#include "mp-instance.h"

G_BEGIN_DECLS

#define MP_TYPE_DETAILS_DIALOG (mp_details_dialog_get_type ())
G_DECLARE_FINAL_TYPE (MpDetailsDialog, mp_details_dialog, MP, DETAILS_DIALOG, GtkDialog)

MpDetailsDialog *mp_details_dialog_new           (MpClient   *client,
                                                  MpInstance *instance);

G_END_DECLS
