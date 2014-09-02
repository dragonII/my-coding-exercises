from django.contrib import admin
from projects.models import Project, P1_Status, Customer 

# Register your models here.
admin.site.register(Project)
#admin.site.register(Employee)
admin.site.register(Customer)
admin.site.register(P1_Status)

