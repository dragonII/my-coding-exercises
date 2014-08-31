from django.db import models

# Create your models here.
class Person(models.Model):
    name = models.CharField(max_length = 100, verbose_name = "full name")

class Car(models.Model):
    name = models.CharField(max_length = 100)
    price = models.DecimalField(max_digits=5, decimal_places = 2)
    doc = models.FileField(upload_to = 'cars')
