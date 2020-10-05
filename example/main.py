import os
import numpy as np
import rohm

from flask import Flask
from flask import render_template, send_file, jsonify
from flask import request

app = Flask('ROHM', static_url_path='')
LAST_BOUNDS = ()

@app.route("/")
def index():
    return render_template("main.html")

@app.route("/path")
def path():
    return ""

@app.route("/estimate")
def estimate():
    global LAST_BOUNDS

    start = np.array([ 40.142727, -105.101341 ])
    nw = start + np.array([ 1.6, -1.6 ])
    se = start + np.array([ -1.6, 1.6 ])

    theta = np.random.uniform() * (3.14159 * 2)

    trip = [(start[0], start[1], 60)]
    
    for i in range(40000):
        theta += np.random.normal(0, 0.01)
        wp = trip[-1]
        trip.append((wp[0] + 0.0001 * np.cos(theta), wp[1] + 0.0001 * np.sin(theta), 60))

    nw, se = rohm.window_from_path(trip)
    LAST_BOUNDS = (nw, se)

    d_lat = int((nw[0] - se[0]) * 100)
    d_lng = int((se[1] - nw[1]) * 100)

    print(d_lat, d_lng)

    estimated = rohm.estimate_path(
        trip=trip,
        size=(d_lng, d_lat),
        mass_kg=2000,
        avg_kwh_km=16,
        regen_efficiency=0.12,
        energy_kwh=24)

    reduced = []
    for i in range(len(estimated) // 60):
        reduced.append(estimated[i * 60])

    return jsonify(reduced)


@app.route("/heatmap")
def heatmap():
    return send_file('/tmp/rohmxzy.tif')

if __name__ == '__main__':
    port = 8080
    if 'PORT' in os.environ:
        port = os.environ['PORT']

    try:
        if os.environ['TEMPLATES_AUTO_RELOAD']:
            app.config['TEMPLATES_AUTO_RELOAD'] = True
    except KeyError:
        print('Not reloading view templates')

    app.run(port=port, host='0.0.0.0')

