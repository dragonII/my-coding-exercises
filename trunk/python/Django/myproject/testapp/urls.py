from django.conf.urls import patterns, url

from testapp import views

urlpatterns = patterns('',
                url(r'^$', views.people, name = 'index'),
                url('(\d+)/', views.people, name = "people_detail"),
                )
