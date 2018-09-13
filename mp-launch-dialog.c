#include "mp-launch-dialog.h"

struct _MpLaunchDialog
{
    GtkDialog  parent_instance;

    GtkEntry  *name_entry;
};

G_DEFINE_TYPE (MpLaunchDialog, mp_launch_dialog, GTK_TYPE_DIALOG)

void
mp_launch_dialog_class_init (MpLaunchDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/ubuntu/multipass/mp-launch-dialog.ui");

    gtk_widget_class_bind_template_child (widget_class, MpLaunchDialog, name_entry);
}

void
mp_launch_dialog_init (MpLaunchDialog *dialog)
{
    gtk_widget_init_template (GTK_WIDGET (dialog));
}

MpLaunchDialog *
mp_launch_dialog_new (void)
{
    return g_object_new (MP_TYPE_LAUNCH_DIALOG, NULL);
}
