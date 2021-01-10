from django.urls import path, include
from rest_framework import routers
from django.contrib.auth import views as auth_views
from . import views


urlpatterns = [
    path('', views.index, name="index"),
    path('cars/add/', views.add),
    path('modal/<str:modal_name>', views.modal, name="modal"),
    path('accounts/login/', auth_views.LoginView.as_view(template_name='login.html'), name='login'),
    path('accounts/logout/', views.logout_view, name='logout'),
    path('accounts/profile/', views.profile),
    # path('accounts/', include('django.contrib.auth.urls')),
]
