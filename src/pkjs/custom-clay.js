module.exports = function(minified) {
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;
	
	function importPMKey() {
		this.set('Getting keys...');
		var email = clayConfig.getItemByMessageKey('pmEmail').get();
		var pin = clayConfig.getItemByMessageKey('pmPIN').get();
		
		$.request('get', 'https://pmkey.xyz/search/?email=' + email + '&pin=' + pin)
		.then(function(responseText) {
			var json = JSON.parse(responseText);
			if (json.success) {
				if (json.keys.weather.owm) {
					clayConfig.getItemByMessageKey('owmKey').set(json.keys.weather.owm);
				}
				if (json.keys.weather.wu) {
					clayConfig.getItemByMessageKey('wuKey').set(json.keys.weather.wu);
				}
				if (json.keys.weather.forecast) {
					clayConfig.getItemByMessageKey('dsKey').set(json.keys.weather.forecast);
				}
			} else {
				clayConfig.getItemById('importButton').set('Error!');
			}
		})
		.error(function(status, statusText, responseText) {
			clayConfig.getItemById('importButton').set('Error!');
		});
	}
	
	function importLatLong() {
		this.set('Getting coordinates...');
		
		var address = clayConfig.getItemByMessageKey('defaultLocation').get();
		
		$.request('get', 'https://maps.googleapis.com/maps/api/geocode/json?address=' + address + '&key=' + "YOUR_KEY_HERE")
		.then(function(responseText) {
			var json = JSON.parse(responseText);
			if (json.status == 'OK') {
				if (json.results[0].geometry.location.lat) {
					clayConfig.getItemByMessageKey('defaultLat').set(json.results[0].geometry.location.lat);
				}
				if (json.results[0].geometry.location.lng) {
					clayConfig.getItemByMessageKey('defaultLong').set(json.results[0].geometry.location.lng);
				}
				clayConfig.getItemById('getLatLongButton').set('Translate to lat/long');
			} else {
				clayConfig.getItemById('getLatLongButton').set('Error!');
			}
		})
		.error(function(status, statusText, responseText) {
			clayConfig.getItemById('getLatLongButton').set('Error!');
		});
	}
	
	function toggleWeather() { // Enable or disable changing settings based on whether weather is enabled
		if (this.get() !== false) {
			clayConfig.getItemByMessageKey('weatherProvider').enable();
			clayConfig.getItemByMessageKey('temperatureUnit').enable();
			clayConfig.getItemByMessageKey('defaultEnabled').enable();
			if (clayConfig.getItemByMessageKey('defaultEnabled').get() === true) {
				clayConfig.getItemByMessageKey('defaultLocation').enable();
				clayConfig.getItemByMessageKey('defaultLat').enable();
				clayConfig.getItemByMessageKey('defaultLong').enable();
				clayConfig.getItemById('getLatLongButton').enable();
			}
		} else {
			clayConfig.getItemByMessageKey('weatherProvider').disable();
			clayConfig.getItemByMessageKey('temperatureUnit').disable();
			clayConfig.getItemByMessageKey('defaultEnabled').disable();
			clayConfig.getItemByMessageKey('defaultLocation').disable();
			clayConfig.getItemByMessageKey('defaultLat').disable();
			clayConfig.getItemByMessageKey('defaultLong').disable();
			clayConfig.getItemById('getLatLongButton').disable();
		}
	}
	
	function toggleLocation() {
		if (this.get() !== false) {
			clayConfig.getItemByMessageKey('defaultLocation').enable();
			clayConfig.getItemByMessageKey('defaultLat').enable();
			clayConfig.getItemByMessageKey('defaultLong').enable();
			clayConfig.getItemById('getLatLongButton').enable();
		} else {
			clayConfig.getItemByMessageKey('defaultLocation').disable();
			clayConfig.getItemByMessageKey('defaultLat').disable();
			clayConfig.getItemByMessageKey('defaultLong').disable();
			clayConfig.getItemById('getLatLongButton').disable();
		}
	}
	
	clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
		var importButton = clayConfig.getItemById('importButton');
		importButton.on('click', importPMKey);
		
		var weatherToggle = clayConfig.getItemByMessageKey('weatherEnabled');
		toggleWeather.call(weatherToggle);
		weatherToggle.on('click', toggleWeather);
		
		var locationToggle = clayConfig.getItemByMessageKey('defaultEnabled');
		toggleLocation.call(locationToggle);
		locationToggle.on('click', toggleLocation);
		
		var getLatLongButton = clayConfig.getItemById('getLatLongButton');
		getLatLongButton.on('click', importLatLong);
	});
	
};