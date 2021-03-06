#include <gst/gst.h>

static gchar *opt_effects = NULL;

//#define DEFAULT_EFFECTS "navigationtest,identity,exclusion,agingtv,videoflip,vertigotv,gaussianblur,shagadelictv,edgetv"
#define DEFAULT_EFFECTS "shagadelictv,vertigotv,exclusion,agingtv,edgetv,navigationtest,videoflip,identity,gaussianblur"

static GstPad *blockpad;
static GstElement *conv_before;
static GstElement *conv_after;
static GstElement *cur_effect;
static GstElement *pipeline;

static GQueue effects = G_QUEUE_INIT;

static GstPadProbeReturn
event_probe_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    GMainLoop *loop = user_data;
    GstElement *next;
    //gboolean ret;
    //GstStateChangeReturn stateret;

    if(GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS)
    {
        g_print("block probe ID: %lu\n", GST_PAD_PROBE_INFO_ID(info));
        return GST_PAD_PROBE_OK;
    }

    /* EOS event received */
    //g_print("Blocking effect's src pad\n");
    gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
    //g_print("BLOCK_PROBE removed after when EOS\n");

    /* push current effect back info the queue */
    //g_queue_push_tail(&effects, gst_object_ref(cur_effect));
    /* take next effect from the queue */
    next = g_queue_pop_head(&effects);
    if(next == NULL)
    {
        GST_DEBUG_OBJECT(pad, "no more effects");
        g_print("\nno more effect on %s\n", GST_OBJECT_NAME(pad));
        g_main_loop_quit(loop);
        return GST_PAD_PROBE_DROP;
    }
    g_print("Next effect: %s\n", GST_OBJECT_NAME(next));

    g_print("Switching from '%s' to '%s'..\n", GST_OBJECT_NAME(cur_effect),
        GST_OBJECT_NAME(next));

    gst_element_set_state(cur_effect, GST_STATE_NULL);

    /* remove unlinks automatically */
    GST_DEBUG_OBJECT(pipeline, "removing %" GST_PTR_FORMAT, cur_effect);
    g_print("removing %s\n", GST_OBJECT_NAME(cur_effect));
    gst_bin_remove(GST_BIN(pipeline), cur_effect);

    GST_DEBUG_OBJECT(pipeline, "adding  %" GST_PTR_FORMAT, next);
    g_print("adding %s\n", GST_OBJECT_NAME(next));
    gst_bin_add(GST_BIN(pipeline), next);
    //if(ret == FALSE)
    //    g_print("Add failed\n");
    //else
    //    g_print("Add successful\n");

    GST_DEBUG_OBJECT(pipeline, "linking...");
    gst_element_link_many(conv_before, next, conv_after, NULL);
    //if(ret == FALSE)
    //    g_print("Link failed\n");
    //else
    //    g_print("Link successful\n");

    gst_element_set_state(next, GST_STATE_PLAYING);
    //stateret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    //g_print("GstStateChangeReturn = %s\n", stateret == GST_STATE_CHANGE_SUCCESS ? "Success" : "Others");

    cur_effect = next;
    GST_DEBUG_OBJECT(pipeline, "done");

    return GST_PAD_PROBE_DROP;
}


static gboolean
bus_cb(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    GMainLoop *loop = user_data;

    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_ERROR:
        {
            GError *err = NULL;
            gchar *dbg;

            gst_message_parse_error(msg, &err, &dbg);
            gst_object_default_error(msg->src, err, dbg);
            g_error_free(err);
            g_free(dbg);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

static GstPadProbeReturn
pad_probe_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    GstPad *srcpad, *sinkpad;

    //GST_DEBUG_OBJECT(pad, "pad is blocked now");
    //g_print("%s is blocked now\n", GST_OBJECT_NAME(pad));

    /* remove the probe first */
    //g_print("Removing block probe...\n");
    gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
    //g_print("BLOCK_PROBE removed\n");

    /* install new probe for EOS */
    srcpad = gst_element_get_static_pad(cur_effect, "src");
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK |
                              GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
                              event_probe_cb, user_data, NULL);
    gst_object_unref(srcpad);

    /* Push EOS into the element, the probe will be fired when the
     * EOS leaves the effect and it has thus drained all of its data */
    sinkpad = gst_element_get_static_pad(cur_effect, "sink");
    gst_pad_send_event(sinkpad, gst_event_new_eos());
    gst_object_unref(sinkpad);

    return GST_PAD_PROBE_OK;
}

static gboolean
timeout_cb(gpointer user_data)
{
    //GstStateChangeReturn ret;

    //g_print("\nIn timeout_cb\n");
    gst_element_get_state (pipeline, NULL, NULL, -1);
    //g_print("GstStateChangeReturn: %d\n", ret);
    //if (ret == GST_STATE_CHANGE_FAILURE) {
    //    g_print ("failed to play the file\n");
    //}
    gst_pad_add_probe(blockpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
            pad_probe_cb, user_data, NULL);

    return TRUE;
}

int main(int argc, char **argv)
{
    GOptionEntry options[] = 
    {
        {"effects", 'e', 0, G_OPTION_ARG_STRING, &opt_effects,
            "Effects to use (comma-separated list of element names)", NULL},
        {NULL}
    };
    GOptionContext *ctx;
    GError *err = NULL;
    GMainLoop *loop;
    GstElement *src, *sink;
    gchar **effect_names, **e;

    ctx = g_option_context_new("");
    g_option_context_add_main_entries(ctx, options, NULL);
    g_option_context_add_group(ctx, gst_init_get_option_group());
    if(!g_option_context_parse(ctx, &argc, &argv, &err))
    {
        g_print("Error initializing: %s\n", err->message);
        return 1;
    }

    g_option_context_free(ctx);

    if(opt_effects != NULL)
        effect_names = g_strsplit(opt_effects, ",", -1);
    else
        effect_names = g_strsplit(DEFAULT_EFFECTS, ",", -1);

    for(e = effect_names; e != NULL && *e != NULL; ++e)
    {
        GstElement *el;
        el = gst_element_factory_make(*e, NULL);
        if(el)
        {
            g_print("Adding effect '%s'\n", *e);
            g_queue_push_tail(&effects, el);
        }
    }

    pipeline = gst_pipeline_new("pipeline");

    src = gst_element_factory_make("videotestsrc", NULL);
    g_object_set(src, "is-live", TRUE, NULL);

    blockpad = gst_element_get_static_pad(src, "src");

    conv_before = gst_element_factory_make("videoconvert", NULL);

    cur_effect = g_queue_pop_head(&effects);

    conv_after = gst_element_factory_make("videoconvert", NULL);

    sink = gst_element_factory_make("ximagesink", NULL);

    gst_bin_add_many(GST_BIN(pipeline), src, conv_before, cur_effect, 
                    conv_after, sink, NULL);

    gst_element_link_many(src, conv_before, cur_effect, conv_after, sink, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    loop = g_main_loop_new(NULL, FALSE);

    gst_bus_add_watch(GST_ELEMENT_BUS(pipeline), bus_cb, loop);

    g_timeout_add_seconds(2, timeout_cb, loop);

    g_main_loop_run(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
