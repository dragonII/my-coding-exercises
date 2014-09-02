from django.db import models
from django.utils.translation import ugettext as _
from django.utils.translation import ugettext_lazy as l_
from django.contrib.auth.models import User

# Create your models here.

class Employee(models.Model):
    username = models.CharField(max_length = 30)
    first_name = models.CharField(max_length = 30)
    last_name = models.CharField(max_length = 30)
    email = models.CharField(max_length = 75)
    phone = models.CharField(max_length = 75)
    date_joined = models.DateField('date joined')
    is_active = models.BooleanField(default = True)
    
    def __unicode__(self):
        return self.username;
    
    class Meta:
        db_table = 'employees'



class Customer(models.Model):
    name = models.CharField(max_length = 200)
    address = models.CharField(max_length = 200)
    tel = models.CharField(max_length = 100)

    def __unicode__(self):
        return self.name

    class Meta:
        db_table = 'customers'



class P1_Status(models.Model):
    #project = models.OneToOneField(Project)
    name        = models.CharField('Project Name', max_length = 50)
    description = models.TextField()
    start_date  = models.DateField()
    finish_date = models.DateField()
    doc         = models.FileField('Document', upload_to = "p1/%Y%m%d", max_length = 200)
    #owner_id = models.ForeignKey(Employee)

    def __unicode__(self):
        return self.name
    class Meta:
        db_table = 'p1_status'


class P2_Status(models.Model):
    class Meta:
        db_table = 'p2_status'
    pass

class Project(models.Model):
    id           = models.IntegerField(primary_key = True)
    name         = models.CharField(max_length = 200)
    start_date   = models.DateField('date created')
    deliver_date = models.DateField('date delivered')
    customer     = models.ForeignKey(Customer)
    owner        = models.ForeignKey(User)
    p1           = models.ForeignKey(P1_Status)

    def __unicode__(self):
        return self.name

    class Meta:
        db_table = 'projects'


