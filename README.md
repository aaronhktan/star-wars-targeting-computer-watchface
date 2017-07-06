# Star Wars Targeting Computer Watchface
A Star Wars Targeting Computer watchface for Pebble, written in C.
**Featured on the Pebble App Store!**

![alt tag](https://github.com/cheeseisdisgusting/star-wars-targeting-computer-watchface/blob/master/screenshots/Banner.png)

It's no good, I can't maneuver! At least with this watchface, you'll be able to tell the time. Featuring the Star Wars Targeting Computer, this watchface includes the time, date, step count (at the bottom), and battery percentage (at the top).

May the Force be with you.

You can find it on the App Store [here](https://goo.gl/ataS3J), and as always, if you like it, give it a heart.

- - - - -

#### Important instructions for cloning!

The default OpenWeatherMap key is not kept in this repository. To use your own, create a "keys.js" file in /src/pkjs if local, or add a new JavaScript file called "keys.js" by clicking the "Add new" button next to the App Source in CloudPebble. This file should contain:
```javascript
module.exports = {
	owmKey: "YOUR_KEY_HERE",
};
```
You can get your free API key at [openweathermap.org]().
