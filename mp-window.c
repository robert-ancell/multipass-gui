#include "mp-window.h"

struct _MpWindow
{
    GtkWindow parent_instance;
};

G_DEFINE_TYPE (MpWindow, mp_window, GTK_TYPE_WINDOW)

void
mp_window_class_init (MpWindowClass *klass)
{
}

void
mp_window_init (MpWindow *window)
{
}

MpWindow *
mp_window_new (void)
{
    return g_object_new (MP_TYPE_WINDOW, NULL);
}
