var Clay = require('pebble-clay');
var clayConfig = require('./config');
var customClay = require('./custom-clay');
var messageKeys = require('message_keys');
var clay = new Clay(clayConfig, customClay);

var weather =  require('./weather.js');

Pebble.addEventListener('ready', function (e) {
		console.log('PebbleKit JS ready!');
		weather.getWeather();
	}
);

Pebble.addEventListener('appmessage', function (e) {
	var settings = JSON.parse(localStorage.getItem('clay-settings'));
	if (settings.weatherEnabled || settings === null) {
		weather.getWeather();
	}
});