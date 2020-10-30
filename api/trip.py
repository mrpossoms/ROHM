from httplib2 import Http
import json
import polyline
API_KEY='AIzaSyAKwDXQvVOVmEfgRHsT4ah6uEfNC76nymE'

class Leg(object):
    """docstring for trip"""
    def __init__(self, origin_str, dest_str):
        super(Leg, self).__init__()
        self.origin = origin_str.replace(' ', ',')
        self.dest = dest_str.replace(' ', ',')

    def host_str(self):
        return 'maps.googleapis.com'

    def request_str(self):
        global API_KEY
        return 'maps/api/directions/json?origin={}&destination={}&key={}'.format(self.origin, self.dest, API_KEY)

    def waypoints(self):
        http = Http()
        url = 'https://{}/{}'.format(self.host_str(), self.request_str())
        resp, content = http.request(url, 'GET')
        json_obj = json.loads(content)
        points = polyline.decode(json_obj['routes'][0]['overview_polyline']['points'])
        return points

    def __str__(self):
        return 'Leg{} - origin: "{}", destination: "{}"'.format(id(self), self.origin, self.dest)


if __name__ == '__main__':
    print(Leg('Longmont CO', 'Lyons CO').waypoints())
