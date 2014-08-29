from django.shortcuts import render
from testapp.models import Person
from django_tables2 import RequestConfig
from testapp.tables import PersonTable
from django import forms

# Create your views here.

def people(request):
    table = PersonTable(Person.objects.all())
    people = Person.objects.all()
    RequestConfig(request, paginate = {'per_page': 15}).configure(table)
    return render(request, "people.html", {"table": table, "people": people})

class UploadFileForm(forms.Form):
    title = forms.CharField(max_length = 50)
    file  = forms.FileField()

from django.core.files.uploadedfile import SimpleUploadedFile
def file_upload(request):
    if request.method == 'POST':
        form = UploadFileForm(request.POST, request.FILES)
        print "ABCDEFG %s" % request.FILES['file']
        if form.is_valid():
            print request.FILES['file']
            instance = ModelWithFileField(file_field = request.FILES['file'])
            instance.save()
            return HttpResponseRedirect('/people/')
    else:
        form = UploadFileForm()
    return render(request, 'upload.html', {'form': form})

def upload(request):
    return render(request, 'upload.html')
