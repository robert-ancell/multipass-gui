/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "mp-application.h"
#include "mp-window.h"

struct _MpApplication
{
    GtkApplication parent_instance;
};

G_DEFINE_TYPE (MpApplication, mp_application, GTK_TYPE_APPLICATION)

static void
mp_application_startup (GApplication *application)
{
    G_APPLICATION_CLASS (mp_application_parent_class)->startup (application);
}

static void
mp_application_activate (GApplication *application)
{
    MpWindow *window = mp_window_new ();
    gtk_application_add_window (GTK_APPLICATION (application), GTK_WINDOW (window));
    gtk_window_present (GTK_WINDOW (window));
}

void
mp_application_class_init (MpApplicationClass *klass)
{
    GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

    application_class->startup = mp_application_startup;
    application_class->activate = mp_application_activate;
}

void
mp_application_init (MpApplication *application)
{
}

MpApplication *
mp_application_new (void)
{
    return g_object_new (MP_TYPE_APPLICATION,
                         "application-id", "com.ubuntu.multipass-gui",
                         NULL);
}
