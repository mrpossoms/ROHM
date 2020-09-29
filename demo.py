import rohm
import numpy as np

start = np.array([ 40.142727, -105.101341 ])
nw = start + np.array([ 1.6, -1.6 ])
se = start + np.array([ -1.6, 1.6 ])

theta = np.random.uniform() * (3.14159 * 2)

trip = [(start[0], start[1], 60)]

for i in range(20000):
	theta += np.random.normal(0, 0.01)
	wp = trip[-1]
	trip.append((wp[0] + 0.0001 * np.cos(theta), wp[1] + 0.0001 * np.sin(theta), 60))

nw, se = rohm.window_from_path(trip)

d_lat = int((nw[0] - se[0]) * 100)
d_lng = int((se[1] - nw[1]) * 100)

print(d_lat, d_lng)

rohm.estimate_path(
	trip=trip,
	size=(d_lng, d_lat),
	mass_kg=2000,
	avg_kwh_km=16,
	regen_efficiency=0.12,
	energy_kwh=24)