#include "mp-client.h"
#include "mp-window.h"

int
main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    g_autoptr(MpClient) client = mp_client_new();
    mp_client_list_async (client, NULL, NULL, NULL);

    MpWindow *window = mp_window_new ();
    gtk_widget_show (GTK_WIDGET (window));

    gtk_main ();

    return 0;
}
