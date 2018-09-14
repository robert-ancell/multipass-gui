#pragma once

#include <gtk/gtk.h>

#include "mp-client.h"

G_BEGIN_DECLS

#define MP_TYPE_LAUNCH_DIALOG (mp_launch_dialog_get_type ())
G_DECLARE_FINAL_TYPE (MpLaunchDialog, mp_launch_dialog, MP, LAUNCH_DIALOG, GtkDialog)

MpLaunchDialog *mp_launch_dialog_new            (MpClient       *client);

const gchar    *mp_launch_dialog_get_name       (MpLaunchDialog *dialog);

gchar          *mp_launch_dialog_get_image_name (MpLaunchDialog *dialog);

G_END_DECLS
