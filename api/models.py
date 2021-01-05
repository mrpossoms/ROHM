from django.db import models
from django.contrib.auth.models import User


class Brand(models.Model):
    approved = models.BooleanField(null=True)
    name = models.CharField(max_length=32)
    user = models.ForeignKey(User, on_delete=models.SET_NULL, null=True)
    
    def __str__(self):
        return self.name

class Vehicle(models.Model):
    approved = models.BooleanField(null=True)
    make = models.ForeignKey(Brand, on_delete=models.CASCADE)
    user = models.ForeignKey(User, on_delete=models.SET_NULL, null=True)
    model = models.CharField(max_length=32)
    year = models.PositiveSmallIntegerField()

    energy_kwh = models.FloatField()
    avg_economy_kwh_km = models.FloatField()
    mass_kg = models.FloatField()
    regen_efficiency = models.FloatField()

    def __str__(self):
        return '{} {} {}'.format(self.year, self.make, self.model)
