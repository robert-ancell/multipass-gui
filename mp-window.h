#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MP_TYPE_WINDOW (mp_window_get_type ())
G_DECLARE_FINAL_TYPE (MpWindow, mp_window, MP, WINDOW, GtkWindow)

MpWindow *mp_window_new (void);

G_END_DECLS
