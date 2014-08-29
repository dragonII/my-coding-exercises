from django.conf.urls import patterns, url

from projects import views

urlpatterns = patterns('',
                url(r'^$', views.listing, name = 'index'),
                url('(\d+)/', views.detail, name = "detail"),
                #url(r'^projects', views.listing, name = 'index'),
                #url(r'^(?P<project_id>\d+)/$', views.detail, name = 'detail'),
                #url(r'^list', views.listing, name = 'listing'),
                )
