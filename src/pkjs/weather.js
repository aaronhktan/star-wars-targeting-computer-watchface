var exports = module.exports = {};
var keys = require('./keys.js');
var temperature = 0; 
var conditions = "Cloudy";
var settings, owmKey, temperatureUnit;

var xhrRequest = function(url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function() {
		callback(this.responseText);
	};
	xhr.open(type, url);
	xhr.send();
};

function sendWeather(temperature, conditions) {
	
	if ((Pebble.getActiveWatchInfo().platform != "chalk") && temperature != "ERR") {
		switch(temperatureUnit) {
			case 1:
				temperature += "F";
				break;
			default:
				temperature += "C";
				break;
		}
	}
	
	var dictionary = {
		'temperature': String(temperature),
		'conditions': parseInt(conditions)
	};
	
	Pebble.sendAppMessage(dictionary, function (e) {
		console.log("Weather info sent! " + temperature + " " + conditions);
	}, function (e) {
		console.log("Weather info not sent!");
	});
}

function getOWMWeather(position) {
	try {
		var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + position.coords.latitude + '&lon=' + position.coords.longitude + '&appid=' + owmKey;
		xhrRequest(url, 'GET',
							function(responseText) {
								try {
									var json = JSON.parse(responseText);
									
									// Get temperature
									temperature = String(Math.round(json.main.temp - 273.15));
									if (temperatureUnit == "1") {
										temperature = String(Math.round((json.main.temp - 273.15) * 9 / 5) + 32);
									}
									
									// Get icon
									var icon = json.weather[0].icon;
									if (icon == "01d") {
										conditions = 1;
									} else if (icon == "02d" || icon == "50d") {
										conditions = 2;
									} else if (icon == "03d" || icon == "04d" || icon == "03n" || icon == "04n") {
										conditions = 3;
									} else if (icon == "09d" || icon == "10d" || icon == "09n" || icon == "10n") {
										conditions = 4;
									} else if (icon == "11d" || icon == "11n") {
										conditions = 5;
									} else if (icon == "13d" || icon == "13n") {
										conditions = 6;
									} else if (icon == "01n") {
										conditions = 7;
									} else if (icon == "02n" || icon == "50n") {
										conditions = 8;
									} else {
										conditions = 0;
									}
									
									sendWeather(temperature, conditions);
								} catch (e) {
									sendWeather("ERR", 0);
								}
							});
	} catch (e) {
		sendWeather("ERR", 0);
	}
}

function getWUWeather(position) {
	if (settings.wuKey !== "") {
		var url = 'http://api.wunderground.com/api/' + settings.wuKey + '/conditions/q/' + position.coords.latitude + ',' + position.coords.longitude + '.json';
		xhrRequest(url, 'GET',
							 function(responseText) {
								 try {
									 var json = JSON.parse(responseText);
									 
									 if (temperatureUnit == "1") {
										 temperature = String(Math.round(json.current_observation.temp_f));
									 } else {
										 temperature = String(Math.round(json.current_observation.temp_c));
									 }
									 
									 var icon = json.current_observation.icon_url.split("/").slice(-1)[0];
									 if (icon == "clear.gif" || icon == "sunny.gif" || icon == "hazy.gif") {
										 conditions = 1;
									 } else if (icon == "mostlycloudy.gif" || icon == "partlycloudy.gif" || icon == "partlysunny.gif") {
										 conditions = 2;
									 } else if (icon == "cloudy.gif") {
										 conditions = 3;
									 } else if (icon == "rain.gif" || icon == "sleet.gif" || icon == "nt_rain.gif" || icon == "nt_sleet.gif") {
										 conditions = 4;
									 } else if (icon == "tstorms.gif" || icon == "nt_tstorms.gif") {
										 conditions = 5;
									 } else if (icon == "flurries.gif" || icon == "snow.gif" || icon == "nt_flurries.gif" || icon == "nt_snow.gif") {
										 conditions = 6;
									 } else if (icon == "nt_clear.gif") {
										 conditions = 7;
									 } else if (icon == "nt_partlycloudy.gif" || icon == "nt_mostlycloudy.gif") {
										 conditions = 8;
									 } else {
										 conditions = 0;
									 }
									 sendWeather(temperature, conditions);
								 } catch (e) {
									 sendWeather("ERR", 0);
								 }
							 });
	} else {
		sendWeather("ERR", 0);
	}
}

function getDSWeather(position) {
	if (settings.dsKey !== "") {
		var url = 'https://api.darksky.net/forecast/' + settings.dsKey + '/' + position.coords.latitude + ',' + position.coords.longitude;
		xhrRequest(url, 'GET',
							 function(responseText) {
								 try {
									 
									 var json = JSON.parse(responseText);
									 if (temperatureUnit == "1") {
										 temperature = String(Math.round(json.currently.temperature));
									 } else {
										 temperature = String(Math.round((json.currently.temperature - 32) * 5 / 9));
									 }
									 
									 var icon = json.currently.icon;
									 if (icon == "clear-day") {
										 conditions = 1;
									 } else if (icon == "partly-cloudy-day") {
										 conditions = 2;
									 } else if (icon == "cloudy" || icon == "fog") {
										 conditions = 3;
									 } else if (icon == "rain" || icon == "sleet") {
										 conditions = 4;
									 } else if (icon == "snow") {
										 conditions = 6;
									 } else if (icon == "clear-night") {
										 conditions = 7;
									 } else if (icon == "partly-cloudy-night") {
										 conditions = 8;
									 } else {
										 conditions = 0;
									 }
									 sendWeather(temperature, conditions);
								 } catch (e) {
									 sendWeather("ERR", 0);
								 }
							 });
	} else {
		sendWeather("ERR", 0);
	}
}

function locationSuccess(position) {
	if (localStorage.getItem('clay-settings') !== null) {
		switch (parseInt(settings.weatherProvider)) {
			case 1:
				getWUWeather(position);
				break;
			case 2:
				getDSWeather(position);
				break;
			default:
				getOWMWeather(position);
				break;
		}
	} else {
		getOWMWeather(position);
	}
}

function locationError(error) {
	sendWeather("ERR", 0);
}

exports.getWeather = function() {
	if (localStorage.getItem('clay-settings') !== null) {
		settings = JSON.parse(localStorage.getItem('clay-settings'));
		if (settings.weatherEnabled === true) {
			navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {timeout: 15000, maximumAge: 60000});
			owmKey = (settings.owmKey === "") ? keys.owmKey : settings.owmKey;
			temperatureUnit = parseInt(settings.temperatureUnit);
		}
	} else {
		navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {timeout: 15000, maximumAge: 60000});
		owmKey = keys.owmKey;
		temperatureUnit = 0;
	}
};