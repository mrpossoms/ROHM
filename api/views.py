from django.shortcuts import render
from django.core import serializers
from django.http import HttpResponse
from rest_framework import viewsets, permissions
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework.decorators import api_view
from .models import Brand, Vehicle, VehicleAddForm
from .serializers import VehicleSerializer, BrandSerializer

import numpy as np
import rohm
import json
import log
from .trip import Leg


class BrandViewSet(viewsets.ModelViewSet):
    # permission_classes = [permissions.IsAuthenticated]
    serializer_class = BrandSerializer

    # def get_serializer_class(self):
    #     return BrandSerializer(self.get_queryset(), many=True)

    def get_queryset(self):
        queryset = Brand.objects.filter(approved=True)
        return queryset

    # def list(self, request):

    # 	serializer = BrandSerializer(queryset, many=True)
    # 	return Response(serializer.data)


class VehicleViewSet(viewsets.ModelViewSet):
    # permission_classes = [permissions.IsAuthenticated]
    serializer_class = VehicleSerializer

    def get_queryset(self):
        queryset = Vehicle.objects.filter(approved=True)
        return queryset

    # def list(self, request):
    # 	queryset = Vehicle.objects.filter(approved=True)
    # 	serializer = VehicleSerializer(queryset, many=True, context={'request': request})
    # 	return Response(serializer.data)

# def brand_list(request):
#     queryset = Brand.objects.filter(approved=True)
#     return HttpResponse(serializers.serialize('json', queryset), content_type="application/json")


# def brand_get(request, brand_pk):
#     queryset = Brand.objects.filter(approved=True)
#     return HttpResponse(serializers.serialize('json', queryset), content_type="application/json")


@api_view()
def brand_vehicle_list(request, brand_str):
    queryset = Vehicle.objects.filter(approved=True).filter(make__name = brand_str)
    # serializer = VehicleSerializer(queryset, many=True, context={'request': request})
    # return Response(serializer.data)
    return HttpResponse(serializers.serialize('json', queryset), content_type="application/json")

@api_view()
def car_add(request):
    if request.method == 'POST':
        form = VehicleAddForm(request.POST)

        if form.is_valid():
            pass
        else:


    Brand.objects.filter(name=)
    return HttpResponse(serializers.serialize('json', queryset), content_type="application/json")


def estimate(request, origin, dest):
    global ALL_CARS

    leg = Leg(origin, dest)
    trip = leg.waypoints()

    for i in range(len(trip)):
        trip[i] += (60,)

    car = {
        'mass_kg': 1536.36,
        'avg_kwh_km': 5.51,
        'regen_efficiency': 0.12,
        'energy_kwh': 24,
    }

    # try to load
    try:
        if 'car' in request.cookies:
            car = ALL_CARS[request.cookies['car']]
    except:
        return error_response('The car specified isn\'t in our records!', 409)


    for key in car:
        try:
            car[key] = float(request.cookies.get(key))
        except:
            if 'car' in request.cookies:
                pass
            else:
                return error_response('Please select a car using the car shaped button, or enter specs for your own!', 410)

    # try to use the provided SoC
    soc = 1
    if 'soc' in request.cookies:
        try:
            soc = float(request.cookies['soc'])
            soc = max(0, min(1, soc))
        except:
            return error_response('There was a problem with the charge level you have set.', 411)

    nw, se = rohm.window_from_path(trip)
    d_lat = max(1, int((nw[0] - se[0]) * 200))
    d_lng = max(1, int((se[1] - nw[1]) * 200))

    try:
        log.info(str(leg))
        estimated = rohm.estimate_path(
            trip=trip,
            size=(d_lng, d_lat),
            mass_kg=car['mass_kg'],
            avg_kwh_km=car['avg_kwh_km'],
            regen_efficiency=car['regen_efficiency'],
            energy_kwh=car['energy_kwh'],
            state_of_charge=soc)
    except:
        log.error('A FATAL ERROR OCCURED FOR:')
        log.error(str(leg))
        log.error('WITH COOKIES:')
        log.error(str(request.cookies))


    return JsonResponse(jsonify(estimated))
