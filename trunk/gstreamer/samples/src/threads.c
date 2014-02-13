#include <gst/gst.h>
#include <pthread.h>

typedef struct
{
    pthread_t thread;
} TestRTId;

G_DEFINE_TYPE (TestRTPool, test_rt_pool, GST_TYPE_TASK_POOL);

static void
default_prepare(GstTaskPool *pool, GError **error)
{
    /* we don't do anything here. We could construct a pool of threads here that
     * we could reuse later but we don't */
}

static void
default_cleanup(GstTaskPool *pool)
{
}

static gpointer
default_push(GstTaskPool *pool, GstTaskPoolFunction func, gpointer data,
             GError **error)
{
    TestRTId *tid;
    gint res;
    pthread_attr_t attr;
    struct sched_param param;

    tid = g_slice_new0(TestRTId);

    pthread_attr_init(&attr);
    if((res = pthread_attr_setschedpolicy(&attr, SCHED_RR)) != 0)
        g_warning("setschedpoliby: failure: %p", g_strerror(res));

    param.sched_priority = 50;
    if((res = pthread_attr_setschedparam(&attr, &param)) != 0)
        g_warning("setschedparam: failure: %p", g_strerror(res));

    if((res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) != 0)
        g_warning("setinheritsched: failure: %p", g_strerror(res));

     res = pthread_create(&tid->thread, &attr, (void *(*)(void *))func, data);

     if(res != 0)
     {
         g_set_error(error, G_THREAD_ERROR, G_THREAD_ERROR_AGAIN,
                    "Error creating thread: %s", g_strerror(res));
         g_slice_free(TestRTId, tid);
         tid = NULL;
     }

     return tid;
}

static void default_join(GstTaskPool *pool, gpointer id)
{
    TestRTId *tid = (TestRTId *)id;

    pthread_join(tid->thread, NULL);

    g_slice_free(TestRTId, tid);
}

static void
test_rt_pool_class_init(TestRTPoolClass *klass)
{
    GstTaskPoolClass *gsttaskpool_class;

    gsttaskpool_class = (GstTaskPoolClass *)klass;

    gsttaskpool_class->prepare = default_prepare;
    gsttaskpool_class->cleanup = default_cleanup;
    gsttaskpool_class->push = default_push;
    gsttaskpool_class->join = default_join;
}

static void test_rt_pool_init(TestRTPool *pool)
{
}

GstTaskPool *
test_rt_pool_new(void)
{
    GstTaskPool *pool;
    pool = g_object_new(TEST_TYPE_RT_POOL, NULL);
    return pool;
}
