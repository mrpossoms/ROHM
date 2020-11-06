from django.urls import path, include
# from rest_framework import routers
from rest_framework_nested import routers

from . import views

router = routers.SimpleRouter()
router.register(r'brand', views.BrandViewSet, basename='brand')

brands_router = routers.NestedSimpleRouter(router, r'brand', lookup='brand')
brands_router.register(r'vehicle', views.VehicleViewSet, basename='brand-vehicle')

# router.register(r'brand/<int:brand_pk>/vehicle', views.VehicleViewSet, basename='vehicle'))
# router.register(r'vehicles/', views.VehicleViewSet, basename='vehicle')

urlpatterns = [
    path(r'', include(router.urls)),
    path(r'', include(brands_router.urls)),
    # path('brand', views.brand_list),
    # path('brand/<int:brand_pk>', views.brand_get),
    # path('brand/<int:brand_pk>/vehicle', views.brand_vehicle_list),
    path('<str:origin>/<str:dest>/estimate', views.estimate, name='estimate')
]
