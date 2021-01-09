from django.apps import apps
from django.shortcuts import render
from django.template import loader
from django.contrib.auth import get_user
from django.contrib.auth.decorators import login_required
from .models import VehicleAddForm

Brand = apps.get_model('api', 'Brand')

# Create your views here.
def index(request):
    return render(request, 'app.html', {})


def modal(request, modal_name):
    # TODO: fix this, this is bad
    return render(request, 'modal/{}.html'.format(modal_name), {})


@login_required
def add(request):
    if 'POST' == request.method:
        form = VehicleAddForm(request.POST)

        if form.is_valid():
            data = form.cleaned_data
            # TODO create the car and brand if applicable
            brand_count = len(Brand.objects.filter(name=data['brand']))

            if brand_count == 0:
                # this brand has not yet been added, lets add it

                pass

            print('{} brands with the name {}'.format(brand_count, brand_name))
        else:
            print('Form was invalid')

    return render(request, 'cars-add.html', {})


def profile(request):
    return render(request, 'profile.html', { 'user': get_user(request) })
