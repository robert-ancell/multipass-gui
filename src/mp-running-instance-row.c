/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <glib/gi18n.h>

#include "mp-running-instance-row.h"

struct _MpRunningInstanceRow
{
    GtkListBoxRow  parent_instance;

    GtkLabel      *address_label;
    GtkLabel      *label;
    GtkSpinner    *spinner;

    MpInstance    *instance;
};

G_DEFINE_TYPE (MpRunningInstanceRow, mp_running_instance_row, GTK_TYPE_LIST_BOX_ROW)

static void
mp_running_instance_row_dispose (GObject *object)
{
    MpRunningInstanceRow *row = MP_RUNNING_INSTANCE_ROW (object);

    g_clear_object (&row->instance);

    G_OBJECT_CLASS (mp_running_instance_row_parent_class)->dispose (object);
}

void
mp_running_instance_row_class_init (MpRunningInstanceRowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = mp_running_instance_row_dispose;

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-running-instance-row.ui");

    gtk_widget_class_bind_template_child (widget_class, MpRunningInstanceRow, address_label);
    gtk_widget_class_bind_template_child (widget_class, MpRunningInstanceRow, label);
    gtk_widget_class_bind_template_child (widget_class, MpRunningInstanceRow, spinner);
}

void
mp_running_instance_row_init (MpRunningInstanceRow *row)
{
    gtk_widget_init_template (GTK_WIDGET (row));
}

MpRunningInstanceRow *
mp_running_instance_row_new (void)
{
    return g_object_new (MP_TYPE_RUNNING_INSTANCE_ROW, NULL);
}

void
mp_running_instance_row_set_instance (MpRunningInstanceRow *row, MpInstance *instance)
{
    g_return_if_fail (MP_IS_RUNNING_INSTANCE_ROW (row));

    g_clear_object (&row->instance);
    row->instance = g_object_ref (instance);
    gtk_label_set_label (row->label, mp_instance_get_name (instance));

    const gchar *state = mp_instance_get_state (instance);
    if (g_strcmp0 (state, "STARTING") == 0) {
        gtk_label_set_label (row->address_label, _("Starting…"));
        gtk_spinner_start (row->spinner);
    }
    else if (g_strcmp0 (state, "RESTARTING") == 0) {
        gtk_label_set_label (row->address_label, _("Restarting…"));
        gtk_spinner_start (row->spinner);
    }
    else {
        gtk_label_set_label (row->address_label, mp_instance_get_ipv4 (instance));
        gtk_spinner_stop (row->spinner);
    }
}

MpInstance *
mp_running_instance_row_get_instance (MpRunningInstanceRow *row)
{
    g_return_val_if_fail (MP_IS_RUNNING_INSTANCE_ROW (row), NULL);
    return row->instance;
}
