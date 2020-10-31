from django.db import models



class Brand(models.Model):
    approved = models.BooleanField(null=True)
    name = models.CharField(max_length=32)

    def __str__(self):
        return self.name

class Vehicle(models.Model):
    approved = models.BooleanField(null=True)
    make = models.ForeignKey(Brand, on_delete=models.CASCADE)
    model = models.CharField(max_length=32)
    year = models.PositiveSmallIntegerField()

    energy_kwh = models.FloatField()
    avg_economy_kwh_km = models.FloatField()
    mass_kg = models.FloatField()
    regen_efficiency = models.FloatField()

    def __str__(self):
        return '{} {} {}'.format(self.year, self.make, self.model)
