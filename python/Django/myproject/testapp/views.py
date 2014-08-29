from django.shortcuts import render
from testapp.models import Person
from django_tables2 import RequestConfig
from testapp.tables import PersonTable

# Create your views here.

def people(request):
    table = PersonTable(Person.objects.all())
    people = Person.objects.all()
    RequestConfig(request, paginate = {'per_page': 15}).configure(table)
    return render(request, "people.html", {"table": table, "people": people})
