module.exports = [
	{
		"type": "heading",
		"defaultValue": "Configuration Page"
	},
	{
		"type": "text",
		"defaultValue": "This is the settings page for the Star Wars Targeting Computer watchface. You are running version 1.5.",
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
			 	"defaultValue": "General Settings"
			},
			{
				"type": "toggle",
				"messageKey": "disconnectEnabled",
				"defaultValue": true,
				"label": "Vibrate on Bluetooth disconnect",
			},
			{
				"type": "toggle",
				"messageKey": "hourlyVibrationEnabled",
				"defaultValue": true,
				"label": "Hourly Vibration",
			},
			{
				"type": "toggle",
				"messageKey": "animateOnShake",
				"defaultValue": true,
				"label": "Animate on shake",
			}
		]
	},
	{
		"type": "section",
		"capabilities": ["NOT_PLATFORM_APLITE"],
		"items": [
			{
				"type": "heading",
			 	"defaultValue": "Health"
			},
			{
				"type": "toggle",
				"messageKey": "healthEnabled",
				"description": "If disabled, step count will not be shown on the watch face.",
				"defaultValue": true,
				"label": "Enable Health",
			},
		]
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
			 	"defaultValue": "Weather"
			},
			{
				"type": "toggle",
				"messageKey": "weatherEnabled",
				"defaultValue": true,
				"label": "Enable Weather",
			},
			{
				"type": "select",
				"messageKey": "weatherProvider",
				"defaultValue": "0",
				"description": "Weather Underground and Dark Sky/Forecast.io require your own API key.",
				"label": "Weather Source",
				"options": [
					{
						"label": "OpenWeatherMap",
						"value": "0"
					},
					{
						"label": "Weather Underground",
						"value": "1"
					},
					{
						"label": "Dark Sky/Forecast.io",
						"value": "2"
					}
				]
			},
			{
				"type": "select",
				"messageKey": "temperatureUnit",
				"defaultValue": "0",
				"label": "Temperature Unit",
				"options": [
					{
						"label": "Celsius",
						"value": "0"
					},
					{
						"label": "Fahrenheit",
						"value": "1"
					}
				]
			},
		]
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
			 	"defaultValue": "API Keys"
			},
			{
				"type": "input",
				"messageKey": "owmKey",
				"label": "OpenWeatherMap Key",
				"attributes": {
					"placeholder": "Default key is being used"
				}
			},
			{
				"type": "input",
				"messageKey": "wuKey",
				"label": "Weather Underground Key",
				"attributes": {
					"placeholder": "Key required for use"
				}
			},
			{
				"type": "input",
				"messageKey": "dsKey",
				"label": "Dark Sky/Forecast.io Key",
				"attributes": {
					"placeholder": "Key required for use"
				}
			},
		]
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"defaultValue": "Pebble Master Key"
			},
			{
				"type": "text",
				"defaultValue": "Import your API keys for OpenWeatherMap, Weather Underground, and Dark Sky/Forecast.io from <a href=pmkey.xyz>pmkey.xyz</a> here.",
			},
			{
				"type": "input",
				"messageKey": "pmEmail",
				"label": "Email Address",
				"attributes": {
					"placeholder": "e.g. name@url.com",
					"type": "email"
				},
			},
			{
				"type": "input",
				"messageKey": "pmPIN",
				"label": "PIN",
				"attributes": {
					"placeholder": "e.g. 91365",
					"type": "tel",
					"limit": 5
				},
			},
			{
				"type": "button",
				"primary": false,
				"defaultValue": "Import Keys",
				"id": "importButton"
			}
		]
	},
	{
		"type": "submit",
		"defaultValue": "Save Settings"
	},
];