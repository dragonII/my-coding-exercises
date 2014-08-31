from django.conf.urls import patterns, url

from testapp import views

urlpatterns = patterns('',
                url(r'^$', views.people, name = 'index'),
                url(r'^upload/', views.file_upload, name = 'file_upload'),
                url(r'^uploading', views.file_upload, name = 'file_upload'),
                url('(\d+)/', views.people, name = "people_detail"),
                url('^contact/$', views.contact, name = "contact"),
                url('^contact/add', views.contact, name = "contact"),
                )
