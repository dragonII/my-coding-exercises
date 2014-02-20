#include <gst/gst.h>

typedef struct _MyExampleMeta MyExampleMeta;

struct _MyExampleMeta
{
    GstMeta     meta;

    gint        age;
    gchar       *name;
};

GType my_example_meta_api_get_type (void);
#define MY_EXAMPLE_META_API_TYPE (my_example_meta_api_get_type())

#define gst_buffer_get_my_example_meta(b) \
    ((MyExampleMeta *)gst_buffer_get_meta((b), MY_EXAMPLE_META_API_TYPE))

const GstMetaInfo *my_example_meta_get_info(void);
#define MY_EXAMPLE_META_INFO (my_example_meta_get_info())

MyExampleMeta *gst_buffer_add_my_example_meta(GstBuffer *buffer,
                                              gint      age,
                                              const gchar *name);

