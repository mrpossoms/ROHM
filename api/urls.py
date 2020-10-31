from django.urls import path, include
from rest_framework import routers

from . import views

router = routers.DefaultRouter()
router.register(r'brand', views.BrandViewSet, basename='brand')
# router.register(r'brand/<str:brand_str>', views.VehicleViewSet, basename='vehicle')
# router.register(r'vehicles/', views.VehicleViewSet, basename='vehicle')

urlpatterns = [
    path('', include(router.urls)),
    path('brand/<str:brand_str>/vehicle', views.brand_vehicles)
    path('<str:origin>/<str:dest>/estimate', views.estimate, name='estimate')
]
