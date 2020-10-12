var rohm = {
	estimate: {},
	ui: {
		map: {},
		error: {}
	}
};

rohm.estimate = {
	make: (origin, destination) => {
		$.get(origin + "/" + destination + "/estimate", function(data) {
			console.log(data);

			var map = rohm.ui.map.inst;

			for (var i = 0; i < data.length - 1; i++)
			{
				var route_coords = [];
				route_coords.push({lat: data[i][0], lng: data[i][1]}, {lat: data[i+1][0], lng: data[i+1][1]});

				var soc = parseInt(Math.max(0, data[i][2]) * 100);
				var g = parseInt(soc * 2.55).toString(16);
				var r = parseInt((100 - soc) * 2.55).toString(16);
				if (g.length < 2) { g += '0'; }
				if (r.length < 2) { r += '0'; }

				if (i == 0 || i == parseInt(data.length / 2) || soc == 0 || i ==
	        data.length - 2)
				{
					var soc_marker = new google.maps.InfoWindow;
					soc_marker.setPosition({lat: data[i][0], lng: data[i][1]});
					soc_marker.setContent(parseInt(100 * data[i][2]) + '% 🔋' );
					soc_marker.open(map);
					rohm.ui.map.soc_icons.push(soc_marker);
				}

				if (soc <= 0) { break; }

				console.log(data[i][2] + " #" + r + g + "00");

				const route_path = new google.maps.Polyline({
					path: route_coords,
					geodesic: true,
					strokeColor: "#" + r + g + "00",
					strokeOpacity: 1.0,
					strokeWeight: 2,
				});
				route_path.setMap(map);

				rohm.ui.map.route_lines.push(route_path);
			}
		})
		.fail(function(e) {
			rohm.ui.error.show("An error occurred while calculating route");	
		});
	},
	clear: () => {
		var ui = rohm.ui.map;
		for (var i = ui.route_lines.length; i--;)
		{
			ui.route_lines[i].setMap(null);
		}
		ui.route_lines = [];

		for (var i = ui.waypoints.length; i--;)
		{
			ui.waypoints[i].setMap(null);
		}
		ui.waypoints = [];

		for (var i = ui.soc_icons.length; i--;)
		{
			ui.soc_icons[i].setMap(null);
		}
		ui.soc_icons = [];
	}
};


rohm.ui.error = {
	show: (reason) => {
		$('#modal-error-text').text(reason);

		$('#modal-error').modal({
			fadeDuration: 100
		});
	}
};

rohm.ui.map = {
	inst: null,
	route_lines: [],
	waypoints: [],
	soc_icons: [],

	click: (event) => {
		var ui = rohm.ui.map;

		if (ui.waypoints.length >= 2)
		{
			rohm.estimate.clear();
			return;
		}

		var marker = new google.maps.Marker({
			position: event.latLng,
			map: rohm.ui.map.inst,
			title: ui.waypoints.length > 0 ? 'Stop ' + ui.waypoints.length : 'Start'
		});

		marker.position.str = function() {
			return marker.position.lat() + "," + marker.position.lng();
		};

		ui.waypoints.push(marker);

		if (ui.waypoints.length == 2)
		{
			rohm.estimate.make(ui.waypoints[0].position.str(), ui.waypoints[1].position.str());
		}
	},

	init: () => {
		var map = rohm.ui.map.inst = new google.maps.Map(document.getElementById("map"), {
			center: {
				lat: 0,
				lng: 0
			},
			zoom: 8
		});

		map.addListener('click', rohm.ui.map.click);

		if (navigator.geolocation)
		{
			navigator.geolocation.getCurrentPosition(function(position) {
				var pos = {
					lat: position.coords.latitude,
					lng: position.coords.longitude
				};

				map.setCenter(pos);
			},
			function() {
				// handleLocationError(true, infoWindow, map.getCenter());
			});
		}
		else
		{
			// Browser doesn't support Geolocation
			// handleLocationError(false, infoWindow, map.getCenter());
		}

		// handle resize
		window.onresize = function() {$('#map').height(window.innerHeight-100)}
		window.onresize();
	}
};

