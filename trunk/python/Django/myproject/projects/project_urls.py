from django.conf.urls import patterns, url

from prjmanagement import views

urlpatterns = patterns('',
                url(r'^$', views.projects_index, name = 'index'),
                #url(r'^projects', views.index, name = 'index'),
                url(r'^(?P<project_id>\d+)/$', views.detail, name = 'detail'),
                url(r'^list/$', views.listing, name = 'listing'),
                )
