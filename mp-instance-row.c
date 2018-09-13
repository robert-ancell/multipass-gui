#include "mp-instance-row.h"

struct _MpInstanceRow
{
    GtkListBoxRow  parent_instance;

    GtkLabel      *label;
};

G_DEFINE_TYPE (MpInstanceRow, mp_instance_row, GTK_TYPE_LIST_BOX_ROW)

void
mp_instance_row_class_init (MpInstanceRowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-instance-row.ui");

    gtk_widget_class_bind_template_child (widget_class, MpInstanceRow, label);
}

void
mp_instance_row_init (MpInstanceRow *row)
{
    gtk_widget_init_template (GTK_WIDGET (row));
}

MpInstanceRow *
mp_instance_row_new (const gchar *name)
{
    MpInstanceRow *row = g_object_new (MP_TYPE_INSTANCE_ROW, NULL);

    gtk_label_set_label (row->label, name);

    return row;
}
