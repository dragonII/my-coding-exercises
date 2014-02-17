#include <gst/gst.h>

GstElement *pipeline, *sink;

static void
cb_pad_added(GstElement *dec,
             GstPad     *pad,
             gpointer   data)
{
    GstCaps *caps;
    GstStructure *str;
    const gchar *name;
    GstPadTemplate *templ;
    GstElementClass *klass;

    /* check media type */
    caps = gst_pad_query_caps(pad, NULL);
    str = gst_caps_get_structure(caps, 0);
    name = gst_structure_get_name(str);

    klass = GST_ELEMENT_GET_CLASS(sink);

    if(g_str_has_prefix(name, "audio"))
    {
        templ = gst_element_class_get_pad_template(klass, "audio_sink");
    } else if(g_str_has_prefix(name, "video"))
    {
        templ = gst_element_class_get_pad_template(klass, "video_sink");
    } else if(g_str_has_prefix(name, "text"))
    {
        templ = gst_element_class_get_pad_template(klass, "text_sink");
    } else
        templ = NULL;

    if(templ)
    {
        GstPad *sinkpad;

        sinkpad = gst_element_request_pad(sink, templ, NULL, NULL);

        if(!gst_pad_is_linked(sinkpad))
        {
            gst_pad_link(pad, sinkpad);
        }
        gst_object_unref(sinkpad);
    }
}

int main(int argc, char **argv)
{
    GMainLoop *loop;
    GstElement *dec;
    GstBus *bus;

    gst_init(&argc, &argv);
    loop = g_main_loop_ew(NULL, FALSE);

    if(argc != 2)
    {
        g_print("Usage: %s <uri>\n", argv[0]);
        return -1;
    }

    /* setup */
    pipeline = gst_pipeline_new("pipeline");

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, my_bus_callback, loop);
    gst_object_unref(bus);

    dec = gst_element_factory_make("uridecodebin", "source");
    g_object_set(G_OBJECT(dec), "uri", argv[1], NULL);
    g_signal_connect(dec, "pad-added", G_CALLBACK(cb_pad_added), NULL);

    /* create audio output */
    sink = gst_element_factory_make("playsink", "sink");
    gst_util_set_object_arg(G_OBJECT(sink), "flags",
                "soft-colorbalance+soft-volume+vis+text+audio+video");
    gst_bin_add_many(GST_BIN(pipeline), dec, sink, NULL);

    /* run */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_main_loop_run(loop);

    /* cleanup */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));

    return 0;
}
