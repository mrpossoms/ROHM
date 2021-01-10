from django.apps import apps
from django.shortcuts import render, redirect
from django.template import loader
from django.contrib.auth import get_user, logout
from django.contrib.auth.decorators import login_required
from .models import VehicleAddForm

from datetime import datetime as dt

Brand = apps.get_model('api', 'Brand')
Vehicle = apps.get_model('api', 'Vehicle')

# Create your views here.
def index(request):
    return render(request, 'app.html', {})


def modal(request, modal_name):
    # TODO: fix this, this is bad
    return render(request, 'modal/{}.html'.format(modal_name), {})


@login_required
def add(request):
    me = get_user(request)

    if 'POST' == request.method:
        form = VehicleAddForm(request.POST)

        if form.is_valid():
            data = form.cleaned_data

            #
            # Fetch or create the brand
            # 
            brand_match = Brand.objects.filter(name__iexact=data['brand'])
            brand_count = len(brand_match)
            brand = None

            if brand_count == 0:
                print('User: {} added new brand: {}'.format(me, data['brand']))
                brand = Brand(name=data['brand'], user=me, approved=False)
                brand.save()
            else:
                brand = brand_match[0]

            #
            # Fetch or create the car
            #
            vehicle_match = Vehicle.objects.filter(model__iexact=data['model']).filter(make=brand)
            vehicle_count = len(vehicle_match)
            vehicle = None

            if vehicle_count == 0:
                vehicle = Vehicle(
                    make=brand, 
                    user=me, 
                    model=data['model'], 
                    year=data['year'],
                    energy_kwh=data['energy_kwh'],
                    avg_kwh_km=data['avg_kwh_km'],
                    mass_kg=data['mass_kg'],
                    regen_efficiency=data['regen_efficiency']
                    )
                vehicle.save()
                return render(request, 'cars-add.html', { 
                    'message': {
                        'type': 'success',
                        'text': 'Your {} {} {} has been added!'.format(vehicle.year, brand.name, vehicle.model),
                    },
                    'year': dt.today().year
                })
            else:
                vehicle = vehicle_match[0]

            return render(request, 'cars-add.html', { 
                'message': {
                    'type': 'warning',
                    'text': 'The {} {} {} already exists.'.format(vehicle.year, brand.name, vehicle.model),
                },
                'year': dt.today().year
            })
        else:
            return render(request, 'cars-add.html', { 
                'message': {
                    'type': 'error',
                    'text': 'Please correct your entries below and try again.',
                },
                'year': dt.today().year
            })

    return render(request, 'cars-add.html', {'year': dt.today().year})


def profile(request):
    return render(request, 'profile.html', { 'user': get_user(request) })

def logout_view(request):
    logout(request)
    return redirect('/')
