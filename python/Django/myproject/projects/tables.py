import django_tables2 as tables
from projects.models import Project
from django_tables2.utils import A

class ProjectTable(tables.Table):
    name = tables.LinkColumn('prj:detail', args = [A('pk')])
    id = tables.LinkColumn('prj:detail', args = [A('pk')])
    owner = tables.LinkColumn('prj:owner_prjs', args = [A('owner_id')])
    customer = tables.LinkColumn('prj:customer_prjs', args = [A('customer_id')])
    p1 = tables.LinkColumn('prj:p1_detail', args = [A('p1_id')])
    #p1 = tables.Column(visible = False)
    p1_date = tables.BooleanColumn(verbose_name = "finished", accessor = A('p1.finished'))
    #owner = tables.LinkColumn('prj:owner_prjs', args = [A('pk')])
    class Meta:
        model = Project
        attrs = {"class": "paleblue"}
