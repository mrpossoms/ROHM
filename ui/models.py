from django.db import models
from django import forms

class VehicleAddForm(forms.Form):
    brand = forms.CharField(label="Brand", max_length=32)
    model = forms.CharField(max_length=32)
    year = forms.IntegerField()

    energy_kwh = forms.FloatField()
    avg_kwh_km = forms.FloatField()
    mass_kg = forms.FloatField()
    regen_efficiency = forms.FloatField()