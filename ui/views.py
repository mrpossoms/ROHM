from django.shortcuts import render
from django.template import loader

# Create your views here.
def index(request):
	return render(request, 'app.html', {})


def modal(request, modal_name):
	# TODO: fix this, this is bad
	return render(request, 'modal/{}.html'.format(modal_name), {})