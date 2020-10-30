var rohm = {
	estimate: {},
	ui: {
		map: {},
		error: {},
		make_slider: null
	},
	API: {}
};


//-----------------------------------------------------------------------------
//    ___    _   _            _       
//   | __|__| |_(_)_ __  __ _| |_ ___ 
//   | _|(_-<  _| | '  \/ _` |  _/ -_)
//   |___/__/\__|_|_|_|_\__,_|\__\___|
//                                    
rohm.estimate = {
	make: (origin, destination) => {
		rohm.API.get("api/" + origin + "/" + destination + "/estimate", (data) => {
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

				if (i == 0 || i == parseInt(data.length / 2) || soc == 0 || i == data.length - 2)
				{
					var soc_marker = new google.maps.InfoWindow;
					soc_marker.setPosition({lat: data[i][0], lng: data[i][1]});
					soc_marker.setContent(parseInt(100 * data[i][2]) + '% 🔋' );
					soc_marker.open(map);
					rohm.ui.map.soc_icons.push(soc_marker);
				}

				if (soc <= 0) { break; }

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
			if (e.responseJSON && e.responseJSON.error)
			{
				rohm.ui.error.show(e.responseJSON.error);
				rohm.estimate.clear();
			}
			else
			{
				rohm.ui.error.show("An error occurred while calculating route");
				rohm.estimate.clear();
			}
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


//-----------------------------------------------------------------------------
rohm.ui.callout = {
	make: (subject_query, message)=>{
		var subject = $(subject_query);
		var body = $('body');
		const sub_pos = subject.position();
		const sub_on_bottom = sub_pos.top > body.height() / 2;
		const sub_on_right = sub_pos.left > body.width() / 2;


		var callout = $('<p></p>')
		.text(message)
		.addClass('rohm-ui-shadow')
		.addClass('rohm-ui-card')
		.appendTo(body);

		var co_left = sub_pos.left + subject.width();
		var co_top = sub_pos.top;

		if (sub_on_right)
		{
			co_left = sub_pos.left - callout.width();		
		}

		callout.css({left: co_left, top: co_top});

		const dismiss = ()=> {
			callout.remove();
		};

		callout.click(dismiss);

		return {
			callout: callout,
			dismiss: dismiss
		};
	}
};

//-----------------------------------------------------------------------------
//         _          _ _    _         
//    _  _(_)      __| (_)__| |___ _ _ 
//   | || | |  _  (_-< | / _` / -_) '_|
//    \_,_|_| (_) /__/_|_\__,_\___|_|  
//                                     
rohm.ui.slider = {
	make: (slider_query, opts) => {
		var holding = false;
		var hold_start_z = 0;
		var start_z = 0;
		var bar = $(slider_query);
		var handle = bar.children('.rohm-slider-handle');

		bar.css({background: 'red'});

		if (opts)
		{
			if (opts.is_vertical) { bar.css({width: '8px'}); }
			else { bar.css({height: '8px'}); }

			if (opts.css) { bar.css(opts.css); }
		}

		const on_down = (event) => { 
			holding = true; 
			hold_start_z = opts.is_vertical ? event.pageY : event.pageX;
			start_z = opts.is_vertical ? handle.position().top : handle.position().left;
		};
		const on_up = (event) => { holding = false; };
		const on_move = (event) => {
			if (!holding) { return; }

			const bar_pos = bar.position();
			const dz = (opts.is_vertical ? event.pageY : event.pageX) - hold_start_z;
			const limit = opts.is_vertical ? bar.height() : bar.width();
			const pos = handle.position();
			const z = start_z + dz;

			if (z < 0 || z > limit) { return; }

			if (opts.is_vertical) { handle.css({left: (bar.width() / 2), top: z}); }
			else { handle.css({top: bar.top, left: z - handle.width() / 2}); }

			const value = z / limit;
			bar.css({background: 'rgb(' + parseInt(255 * (1 - value)) + ',' + parseInt(255 * value) + ', 0)'})

			if (opts && 'function' === typeof(opts.on_change))
			{
				opts.on_change({
					bar: bar,
					handle: handle,
					value: value
				});
			}
		};

		handle
		.mousedown(on_down)
		.on('touchstart', on_down);

		$('body')
		.mouseup(on_up)
		.on('touchend', on_up)
		.mousemove(on_move)
		.on('touchmove', on_move);

		return {
			bar: bar,
			handle: handle,
			set: (value, default_val) => {
				if (typeof(value) !== 'number') { value = default_val; }

				if (opts.is_vertical) { handle.css({left: (bar.width() / 2), top: bar.height() * value}); }
				else { handle.css({top: bar.top, left: (bar.width() * value) - handle.width() / 2}); }
				bar.css({background: 'rgb(' + parseInt(255 * (1 - value)) + ',' + parseInt(255 * value) + ', 0)'})

				if (opts && 'function' === typeof(opts.on_change))
				{
					opts.on_change({
						bar: bar,
						handle: handle,
						value: value
					});
				}
			}
		};
	}
};

//-----------------------------------------------------------------------------
//         _               _   _     _ _        
//    _  _(_)      __ _ __| |_(_)_ _(_) |_ _  _ 
//   | || | |  _  / _` / _|  _| \ V / |  _| || |
//    \_,_|_| (_) \__,_\__|\__|_|\_/|_|\__|\_, |
//                                         |__/ 
rohm.ui.activity = {
	make: (query) => {
		var canvas = $(query);
		var ctx = canvas[0].getContext('2d');
		var t = 0;
		var ticker;

		var animation = () => {
			ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
			ctx.fillRect(0, 0, 32, 32);

			ctx.fillStyle = '#333';
			ctx.beginPath();
			ctx.arc(16 + Math.sin(6*t) * 10, 16 + Math.cos(6*t) * 10, 2, 0, 3.14159 * 2);
			ctx.closePath();
			ctx.stroke();
			ctx.fill();

			t += 16.0 / 1000.0;
		};

		canvas.hide();

		return {
			start: () => {
				canvas.show();
				ticker = setInterval(animation, 16);
			},
			stop: () => {
				canvas.hide();
				clearInterval(ticker);
				var frames = 60;
				var clear_ticker = setInterval(() => {
					ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
					ctx.fillRect(0, 0, 32, 32);
					frames--;
					if (frames == 0) { clearInterval(clear_ticker); }
				}, 16);

			}
		};
	}
};

//-----------------------------------------------------------------------------
//         _                           
//    _  _(_)      ___ _ _ _ _ ___ _ _ 
//   | || | |  _  / -_) '_| '_/ _ \ '_|
//    \_,_|_| (_) \___|_| |_| \___/_|  
//                                     
rohm.ui.error = {
	show: (reason) => {
		$('#modal-error-text').text(reason);

		$('#modal-error').modal({
			fadeDuration: 100
		});
	}
};

//-----------------------------------------------------------------------------
//         _                       
//    _  _(_)      _ __  __ _ _ __ 
//   | || | |  _  | '  \/ _` | '_ \
//    \_,_|_| (_) |_|_|_\__,_| .__/
//                           |_|   
rohm.ui.map = {
	inst: null,
	route_lines: [],
	waypoints: [],
	soc_icons: [],

	click: (event) => {
		var ui = rohm.ui.map;

		if (ui.route_lines.length > 0)
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
		var map_ele = $('#map');
		map_ele.height(document.body.clientHeight);

		var map = rohm.ui.map.inst = new google.maps.Map(map_ele[0], {
			center: {
				lat: 0,
				lng: 0
			},
			zoom: 8,
			mapTypeControl: false,
			fullscreenControl: false
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

//-----------------------------------------------------------------------------
document.cookie_get = (key) => {
	var cookies = document.cookie.split('; ');

	for (var i in cookies)
	{
		if (cookies[i].indexOf(key) >= 0)
		{
			const cookie = cookies[i].split('=');
			if (cookie[0] === key)
			{
				return cookie[1];
			}
		}
	}

	return undefined;
};

//-----------------------------------------------------------------------------
rohm.API.get = (route, on_success) => {
	{ // start activity indicator
		rohm_activity.start();
	}
	
	return $.get(route, on_success)
	.always(() => {
		// stop activity indicator
		rohm_activity.stop();
	});
};
