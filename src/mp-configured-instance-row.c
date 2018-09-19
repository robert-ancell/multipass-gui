/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "mp-configured-instance-row.h"

struct _MpConfiguredInstanceRow
{
    GtkListBoxRow  parent_instance;

    GtkLabel      *label;
    GtkImage      *image;

    MpInstance    *instance;
};

G_DEFINE_TYPE (MpConfiguredInstanceRow, mp_configured_instance_row, GTK_TYPE_LIST_BOX_ROW)

static void
mp_configured_instance_row_dispose (GObject *object)
{
    MpConfiguredInstanceRow *row = MP_CONFIGURED_INSTANCE_ROW (object);

    g_clear_object (&row->instance);

    G_OBJECT_CLASS (mp_configured_instance_row_parent_class)->dispose (object);
}

void
mp_configured_instance_row_class_init (MpConfiguredInstanceRowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = mp_configured_instance_row_dispose;

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-configured-instance-row.ui");

    gtk_widget_class_bind_template_child (widget_class, MpConfiguredInstanceRow, image);
    gtk_widget_class_bind_template_child (widget_class, MpConfiguredInstanceRow, label);
}

void
mp_configured_instance_row_init (MpConfiguredInstanceRow *row)
{
    gtk_widget_init_template (GTK_WIDGET (row));
}

MpConfiguredInstanceRow *
mp_configured_instance_row_new (void)
{
    return g_object_new (MP_TYPE_CONFIGURED_INSTANCE_ROW, NULL);
}

void
mp_configured_instance_row_set_instance (MpConfiguredInstanceRow *row, MpInstance *instance)
{
    g_return_if_fail (MP_IS_CONFIGURED_INSTANCE_ROW (row));

    g_clear_object (&row->instance);
    row->instance = g_object_ref (instance);
    gtk_label_set_label (row->label, mp_instance_get_name (instance));

    const gchar *state = mp_instance_get_state (instance);
    if (g_strcmp0 (state, "DELETED") == 0)
        gtk_image_set_from_icon_name (row->image, "action-unavailable-symbolic", GTK_ICON_SIZE_BUTTON);
    else
        gtk_image_set_from_icon_name (row->image, "computer-symbolic", GTK_ICON_SIZE_BUTTON);
}

MpInstance *
mp_configured_instance_row_get_instance (MpConfiguredInstanceRow *row)
{
    g_return_val_if_fail (MP_IS_CONFIGURED_INSTANCE_ROW (row), NULL);
    return row->instance;
}
