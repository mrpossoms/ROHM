from .models import Vehicle, Brand
from rest_framework import serializers



class BrandSerializer(serializers.HyperlinkedModelSerializer):
	class Meta:
		model = Brand
		fields = ['pk', 'name']


class VehicleSerializer(serializers.HyperlinkedModelSerializer):
	class Meta:
		model = Vehicle
		fields = ['pk', 'make', 'model', 'year', 'energy_kwh', 'avg_kwh_km', 'mass_kg', 'regen_efficiency']