/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "mp-window.h"

int
main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    MpWindow *window = mp_window_new ();
    gtk_widget_show (GTK_WIDGET (window));

    gtk_main ();

    return 0;
}
