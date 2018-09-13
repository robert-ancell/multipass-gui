#include "mp-instance-row.h"
#include "mp-window.h"

struct _MpWindow
{
    GtkWindow  parent_instance;

    GtkWidget *instances_listbox;
};

G_DEFINE_TYPE (MpWindow, mp_window, GTK_TYPE_WINDOW)

static void
add_row (MpWindow *window, const gchar *name)
{
    MpInstanceRow *row = mp_instance_row_new (name);
    gtk_widget_show (GTK_WIDGET (row));
    gtk_container_add (GTK_CONTAINER (window->instances_listbox), GTK_WIDGET (row));
}

void
mp_window_class_init (MpWindowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-window.ui");

    gtk_widget_class_bind_template_child (widget_class, MpWindow, instances_listbox);
}

void
mp_window_init (MpWindow *window)
{
    gtk_widget_init_template (GTK_WIDGET (window));

    add_row (window, "gallant-moonfish");
    add_row (window, "enriching-mooneye");
}

MpWindow *
mp_window_new (void)
{
    return g_object_new (MP_TYPE_WINDOW, NULL);
}
