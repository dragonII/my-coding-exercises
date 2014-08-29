from django.conf.urls import patterns, include, url

from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    # Examples:
    # url(r'^$', 'myproject.views.home', name='home'),
    # url(r'^blog/', include('blog.urls')),

    #url(r'^projects/', include('prjmanagement.urls')),
    #url(r'^index/', include('prjmanagement.urls')),
    #url(r'^$', include('prjmanagement.index_urls')),
    url(r'^projects/', include('projects.urls', namespace="prj")),
    url(r'^admin/', include(admin.site.urls)),
    url(r'^people/', include('testapp.urls')),
)
