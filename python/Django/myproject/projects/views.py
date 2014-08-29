from django.shortcuts import render, get_object_or_404
from django.http import HttpResponse, Http404
from django.utils.translation import ugettext as _
from projects.models import Project, Employee

from django.core.mail import send_mail

from django import forms

from django_tables2 import RequestConfig
from projects.tables import ProjectTable

import logging
logger = logging.getLogger(__name__)

# Create your views here.

def site_index(request):
    context = '<a href="/projects/">index</a>'
    return HttpResponse(_("Hello, world. You're at the site's %s page.") % context)

def projects_index(request):
    context = '<a href="/projects/">index</a>'
    #subject = 'For django test'
    #message = 'Too see whether it can send the mail'
    #sender = 'dragonIIII@163.com'
    #recipients = ['dragonIIII@163.com']
    #send_mail(subject, message, sender, recipients)
    return render(request, 'projects/index.html')
    #return HttpResponse(_("Hello, world. You're at the projects' %s page.") % context)


def detail(request, project_id):
    print "in detail"
    project = get_object_or_404(Project, pk = project_id)
    return render(request, 'projects/detail.html', {'project': project})
    #return HttpResponse(_("You're looking at the information of project %s." % project_id))


def listing(request):
    table = ProjectTable(Project.objects.all())
    RequestConfig(request).configure(table)
    return render(request, 'projects/listing.html', {'table': table})

def owner_prjs(request, owner_id):
    print "in owner_prjs"
    owner = get_object_or_404(Employee, pk = owner_id)
    projects = owner.project_set.all()
    return render(request, 'projects/owner_prjs.html', {'owner': owner, 'projects': projects})

