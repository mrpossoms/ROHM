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
            print(form.cleaned_data)
            # TODO create the car and brand if applicable
            Brand.objects.filter(name=form.cleaned_data['brand'])
        else:
            print('Form was invalid')

    return render(request, 'cars-add.html', {})


def profile(request):
    return render(request, 'profile.html', { 'user': get_user(request) })