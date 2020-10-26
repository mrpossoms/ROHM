from django.db import models


class Brand(models.Model):
    name_text = models.CharField(max_length=32)

    def __str__(self):
        return self.name_text

class Vehicle(models.Model):
    make = models.ForeignKey(Brand, on_delete=models.CASCADE)
    model_text = models.CharField(max_length=32)
    year_int = models.PositiveSmallIntegerField()

    energy_kwh = models.FloatField()
    avg_economy_kwh_km = models.FloatField()
    mass_kg = models.FloatField()
    regen_efficiency = models.FloatField()

    def __str__(self):
        return '{} {} {}'.format(year_int, make, model_text)
