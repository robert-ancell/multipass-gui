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
