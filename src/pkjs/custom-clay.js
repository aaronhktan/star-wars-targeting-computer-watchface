module.exports = function(minified) {
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;
	
	function importPMKey() {
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
			}
		})
		.error(function(status, statusText, responseText) {
			clayConfig.getItemById('importButton').set('Error!');
		});
	}
	
	clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
		var importButton = clayConfig.getItemById('importButton');
		importButton.on('click', importPMKey);
	});
};