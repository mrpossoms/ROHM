from .models import Vehicle, Brand
from rest_framework import serializers



class BrandSerializer(serializers.HyperlinkedModelSerializer):
	class Meta:
		model = Brand
		fields = ['name']


class VehicleSerializer(serializers.HyperlinkedModelSerializer):
	class Meta:
		model = Vehicle
		fields = ['make', 'model', 'year', 'energy_kwh', 'avg_economy_kwh_km', 'mass_kg', 'regen_efficiency']