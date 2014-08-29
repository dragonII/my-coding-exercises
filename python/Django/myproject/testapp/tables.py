import django_tables2 as tables
from testapp.models import Person
from django_tables2.utils import A

class PersonTable(tables.Table):
    name = tables.LinkColumn('people_details', args=[A('pk')])
    class Meta:
        model = Person
        attrs = {"class": "paleblue"}
