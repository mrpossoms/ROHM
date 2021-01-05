from django.urls import path, include
from rest_framework import routers
from django.contrib.auth import views as auth_views
from . import views


urlpatterns = [
    path('', views.index, name="index"),
    path('modal/<str:modal_name>', views.modal, name="modal"),
    path('accounts/login/', auth_views.LoginView.as_view(template_name='login.html')),
    path('accounts/profile/', views.profile),
    # path('accounts/', include('django.contrib.auth.urls')),
]
