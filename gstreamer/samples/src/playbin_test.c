#include <gst/gst.h>

static gboolean
my_bus_callback(GstBus *bus,
                GstMessage *msg,
                gpointer user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    g_print("Got message: %s\n", GST_MESSAGE_TYPE_NAME(msg));
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_ERROR:
        {
            GError *err;
            gchar  *dbg;

            gst_message_parse_error(msg, &err, &dbg);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            g_free(dbg);

            g_main_loop_quit(loop);
            break;
        }
        case GST_MESSAGE_EOS:
            g_main_loop_quit(loop);
            break;
        default:
            break;
    }
    return TRUE;
}


int main(int argc, char **argv)
{
    GMainLoop *loop;
    GstElement *play;
    GstBus *bus;

    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    if(argc != 2)
    {
        g_print("Usage: %s <URI>\n", argv[0]);
        return -1;
    }

    /* setup */
    play = gst_element_factory_make("playbin", "play");
    g_object_set(G_OBJECT(play), "uri", argv[1], NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    gst_bus_add_watch(bus, my_bus_callback, loop);
    gst_object_unref(bus);

    gst_element_set_state(play, GST_STATE_PLAYING);

    /* run */
    g_main_loop_run(loop);

    /* cleanup */
    gst_element_set_state(play, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(play));

    return 0;
}
