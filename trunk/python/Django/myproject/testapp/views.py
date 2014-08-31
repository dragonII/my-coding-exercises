from django.shortcuts import render
from testapp.models import Person
from django_tables2 import RequestConfig
from testapp.tables import PersonTable
from django import forms
from django.http import HttpResponse

# Create your views here.

class ContactForm(forms.Form):
    subject = forms.CharField(max_length = 100)
    message = forms.CharField()
    sender  = forms.EmailField()
    cc_myself = forms.BooleanField(required = False)

def contact(request):
    if request.method == 'POST':
        form = ContactForm(request.POST)
        if form.is_valid():
            print '--------'
            print form.cleaned_data['subject']
            print form.cleaned_data['message']
            print form.cleaned_data['sender']
            print form.cleaned_data['cc_myself']
            print '--------'

            return HttpResponse("Thanks")
    else:
        form = ContactForm()

    return render(request, 'contact.html', {'form':form})

def people(request):
    table = PersonTable(Person.objects.all())
    people = Person.objects.all()
    RequestConfig(request, paginate = {'per_page': 15}).configure(table)
    return render(request, "people.html", {"table": table, "people": people})

class UploadFileForm(forms.Form):
    #title = forms.CharField(max_length = 50)
    doc = forms.FileField()

def handle_uploaded_file(f):
    with open('tmp.txt', 'wb+') as destination:
        for chunk in f.chunks():
            destination.write(chunk)

from django.core.files.uploadedfile import SimpleUploadedFile
def file_upload(request):
    if request.method == 'POST':
        form = UploadFileForm(request.POST, request.FILES)
        #for key in request.FILES.keys():
        #    print "KEY: %s" % key
        if form.is_valid():
            handle_uploaded_file(request.FILES['doc'])
            #instance = ModelWithFileField(file_field = request.FILES['file'])
            #instance.save()
            return HttpResponse('File successfully uploaded')
        else:
            print "Invalid form"
    else:
        form = UploadFileForm()
    return render(request, 'upload.html', {'form': form})

def upload(request):
    return render(request, 'upload.html')
