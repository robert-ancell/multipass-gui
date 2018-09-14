#include "mp-instance-row.h"

struct _MpInstanceRow
{
    GtkListBoxRow  parent_instance;

    GtkLabel      *label;

    gchar         *name;
};

G_DEFINE_TYPE (MpInstanceRow, mp_instance_row, GTK_TYPE_LIST_BOX_ROW)

static void
mp_instance_row_dispose (GObject *object)
{
    MpInstanceRow *row = MP_INSTANCE_ROW (object);

    g_clear_pointer (&row->name, g_free);

    G_OBJECT_CLASS (mp_instance_row_parent_class)->dispose (object);
}

void
mp_instance_row_class_init (MpInstanceRowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = mp_instance_row_dispose;

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

    row->name = g_strdup (name);
    gtk_label_set_label (row->label, name);

    return row;
}

const gchar *
mp_instance_row_get_name (MpInstanceRow *row)
{
    g_return_val_if_fail (MP_IS_INSTANCE_ROW (row), NULL);
    return row->name;
}
