from django.shortcuts import render
from django.template import loader
from django.contrib.auth import get_user
from django.contrib.auth.decorators import login_required

# Create your views here.
def index(request):
	return render(request, 'app.html', {})


def modal(request, modal_name):
	# TODO: fix this, this is bad
	return render(request, 'modal/{}.html'.format(modal_name), {})


@login_required
def add(request):
	# TODO: fix this, this is bad
	return render(request, 'cars-add.html', {})

def profile(request):
	return render(request, 'profile.html', { 'user': get_user(request) })