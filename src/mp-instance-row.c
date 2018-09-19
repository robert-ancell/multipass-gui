/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "mp-instance-row.h"

struct _MpInstanceRow
{
    GtkListBoxRow  parent_instance;

    GtkImage      *image;
    GtkLabel      *label;
    GtkSpinner    *spinner;

    gchar         *name;
    gchar         *state;
};

G_DEFINE_TYPE (MpInstanceRow, mp_instance_row, GTK_TYPE_LIST_BOX_ROW)

static void
mp_instance_row_dispose (GObject *object)
{
    MpInstanceRow *row = MP_INSTANCE_ROW (object);

    g_clear_pointer (&row->name, g_free);
    g_clear_pointer (&row->state, g_free);

    G_OBJECT_CLASS (mp_instance_row_parent_class)->dispose (object);
}

void
mp_instance_row_class_init (MpInstanceRowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = mp_instance_row_dispose;

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-instance-row.ui");

    gtk_widget_class_bind_template_child (widget_class, MpInstanceRow, image);
    gtk_widget_class_bind_template_child (widget_class, MpInstanceRow, label);
    gtk_widget_class_bind_template_child (widget_class, MpInstanceRow, spinner);
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

void
mp_instance_row_set_state (MpInstanceRow *row, const gchar *state)
{
    g_return_if_fail (MP_IS_INSTANCE_ROW (row));
    g_free (row->state);
    row->state = g_strdup (state);

    gtk_widget_show (GTK_WIDGET (row->image));
    gtk_widget_hide (GTK_WIDGET (row->spinner));
    if (strcmp (state, "RUNNING") == 0)
        gtk_image_set_from_icon_name (row->image, "media-playback-start-symbolic", GTK_ICON_SIZE_BUTTON);
    else if (strcmp (state, "STOPPED") == 0)
        gtk_image_set_from_icon_name (row->image, "media-playback-stop-symbolic", GTK_ICON_SIZE_BUTTON);
    else if (strcmp (state, "DELETED") == 0)
        gtk_image_set_from_icon_name (row->image, "action-unavailable-symbolic", GTK_ICON_SIZE_BUTTON);
    else {
        gtk_widget_hide (GTK_WIDGET (row->image));
        gtk_widget_show (GTK_WIDGET (row->spinner));
    }
}

const gchar *
mp_instance_row_get_state (MpInstanceRow *row)
{
    g_return_val_if_fail (MP_IS_INSTANCE_ROW (row), NULL);
    return row->state;
}
