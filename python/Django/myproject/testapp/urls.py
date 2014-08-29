from django.conf.urls import patterns, url

from testapp import views

urlpatterns = patterns('',
                url(r'^$', views.people, name = 'index'),
                url(r'^upload/', views.upload, name = 'upload'),
                url(r'^uploading', views.file_upload, name = 'file_upload'),
                url('(\d+)/', views.people, name = "people_detail"),
                )
