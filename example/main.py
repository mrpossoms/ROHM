import os
import numpy as np
import rohm
import json
import log
from trip import Leg

from flask import Flask
from flask import render_template, send_file, jsonify, url_for
from flask import request, make_response, send_from_directory, redirect

app = Flask('ROHM', static_folder="static")
LAST_BOUNDS = ()
ALL_CARS = json.load(open('data/cars.json', 'r'))

def error_response(message, code):
    resp = make_response(jsonify({'error': message}))
    resp.status_code = code or 400
    return resp

@app.route('/onboarding')
def onboarding():
    resp = make_response(render_template('onboarding.html'))
    resp.set_cookie('onboarded', 'true')
    return resp

@app.route("/")
def index():
    if 'onboarded' not in request.cookies:
        return redirect(url_for('onboarding'))

    return render_template("app.html")

@app.route('/favicon.ico')
def favicon():
    return send_from_directory(os.path.join(app.root_path, 'static/imgs'), 'favicon.ico', mimetype='image/vnd.microsoft.icon')

@app.route("/modal/cars")
def modal_cars():
    return render_template("modal/cars.html")

@app.route("/search/cars/<string:query>")
def search_cars(query):
    global ALL_CARS
    from fuzzysearch import find_near_matches

    matches = []

    # evaluate closeness of string match against all cars
    for car_name in ALL_CARS:
        match = find_near_matches(query, car_name, max_l_dist=1)

        if len(match) > 0:
            matches.append(ALL_CARS[car_name])
            matches[-1]['name'] = car_name
            matches[-1]['dist'] = match[0].dist

    # sort for by 'distance'
    sorted(matches, key=lambda car: car['dist'])

    print(matches)

    return json.dumps(matches)




@app.route("/<string:origin>/<string:dest>/estimate")
def estimate(origin, dest):
    global ALL_CARS

    leg = Leg(origin, dest)
    trip = leg.waypoints()

    for i in range(len(trip)):
        trip[i] += (60,)

    car = {
        'mass_kg': 1536.36,
        'avg_kwh_km': 5.51,
        'regen_efficiency': 0.12,
        'energy_kwh': 24,
    }

    # try to load
    try:
        if 'car' in request.cookies:
            car = ALL_CARS[request.cookies['car']]
    except:
        return error_response('The car specified isn\'t in our records!', 409)


    for key in car:
        try:
            car[key] = float(request.cookies.get(key))
        except:
            if 'car' in request.cookies:
                pass
            else:
                return error_response('Please select a car using the car shaped button, or enter specs for your own!', 410)

    # try to use the provided SoC
    soc = 1
    if 'soc' in request.cookies:
        try:
            soc = float(request.cookies['soc'])
            soc = max(0, min(1, soc))
        except:
            return error_response('There was a problem with the charge level you have set.', 411)

    nw, se = rohm.window_from_path(trip)
    d_lat = max(1, int((nw[0] - se[0]) * 200))
    d_lng = max(1, int((se[1] - nw[1]) * 200))

    try:
        log.info(str(leg))
        estimated = rohm.estimate_path(
            trip=trip,
            size=(d_lng, d_lat),
            mass_kg=car['mass_kg'],
            avg_kwh_km=car['avg_kwh_km'],
            regen_efficiency=car['regen_efficiency'],
            energy_kwh=car['energy_kwh'],
            state_of_charge=soc)
    except:
        log.error('A FATAL ERROR OCCURED FOR:')
        log.error(str(leg))
        log.error('WITH COOKIES:')
        log.error(str(request.cookies))


    return jsonify(estimated)


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

