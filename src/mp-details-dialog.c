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

#include "mp-details-dialog.h"

struct _MpDetailsDialog
{
    GtkDialog     parent_instance;

    GtkLabel     *ipv4_label;
    GtkLabel     *version_label;

    GCancellable *cancellable;
    MpClient     *client;
};

G_DEFINE_TYPE (MpDetailsDialog, mp_details_dialog, GTK_TYPE_DIALOG)

static void
mp_details_dialog_dispose (GObject *object)
{
    MpDetailsDialog *dialog = MP_DETAILS_DIALOG (object);

    g_cancellable_cancel (dialog->cancellable);
    g_clear_object (&dialog->cancellable);
    g_clear_object (&dialog->client);

    G_OBJECT_CLASS (mp_details_dialog_parent_class)->dispose (object);
}

void
mp_details_dialog_class_init (MpDetailsDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = mp_details_dialog_dispose;

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-details-dialog.ui");

    gtk_widget_class_bind_template_child (widget_class, MpDetailsDialog, ipv4_label);
    gtk_widget_class_bind_template_child (widget_class, MpDetailsDialog, version_label);
}

void
mp_details_dialog_init (MpDetailsDialog *dialog)
{
    gtk_widget_init_template (GTK_WIDGET (dialog));
}

MpDetailsDialog *
mp_details_dialog_new (MpClient *client, MpInstance *instance)
{
    MpDetailsDialog *dialog = g_object_new (MP_TYPE_DETAILS_DIALOG,
                                           "use-header-bar", 1,
                                           NULL);

    dialog->cancellable = g_cancellable_new ();
    dialog->client = g_object_ref (client);

    g_autofree gchar *title = g_strdup_printf (_("“%s” Details"), mp_instance_get_name (instance));
    gtk_window_set_title (GTK_WINDOW (dialog), title);

    gtk_label_set_label (dialog->version_label, mp_instance_get_release (instance));
    gtk_label_set_label (dialog->ipv4_label, mp_instance_get_ipv4 (instance));

    return dialog;
}
