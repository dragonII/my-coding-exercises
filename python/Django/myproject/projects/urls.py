from django.conf.urls import patterns, url

from projects import views

urlpatterns = patterns('',
                url(r'^$', views.listing, name = 'index'),
                url('^(\d+)/', views.detail, name = "detail"),
                url(r'^owner/(\d+)/', views.owner_prjs, name = "owner_prjs"),
                url(r'^customer/(\d+)/', views.customer_prjs, name = "customer_prjs"),
                url(r'^p1detail/(\d+)/', views.p1_detail, name = "p1_detail"),
                #url(r'^owner/(?P<owner_id>\d+)/$', views.owner_prjs, name = 'owner_prjs'),
                #url(r'^projects', views.listing, name = 'index'),
                #url(r'^(?P<project_id>\d+)/$', views.detail, name = 'detail'),
                #url(r'^list', views.listing, name = 'listing'),
                )
