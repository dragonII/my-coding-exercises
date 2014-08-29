import django_tables2 as tables
from projects.models import Project
from django_tables2.utils import A

class ProjectTable(tables.Table):
    name = tables.LinkColumn('prj:detail', args = [A('pk')])
    id = tables.LinkColumn('prj:detail', args = [A('pk')])
    class Meta:
        model = Project
        attrs = {"class": "paleblue"}
