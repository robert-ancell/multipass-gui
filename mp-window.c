#include "mp-window.h"

struct _MpWindow
{
    GtkWindow parent_instance;
};

G_DEFINE_TYPE (MpWindow, mp_window, GTK_TYPE_WINDOW)

void
mp_window_class_init (MpWindowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-window.ui");
}

void
mp_window_init (MpWindow *window)
{
    gtk_widget_init_template (GTK_WIDGET (window));
}

MpWindow *
mp_window_new (void)
{
    return g_object_new (MP_TYPE_WINDOW, NULL);
}
