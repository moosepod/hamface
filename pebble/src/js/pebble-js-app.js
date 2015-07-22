var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


/// Band fetching. We fetch bands, then weather after bands completes
/// as there were random errors being thrown when they were being called
/// at the same time. 
function getBands() {
	var dictionary = {
		'KEY_BANDS_DAY': 'fair\ngood\ngood\nfair',
		'KEY_BANDS_NIGHT': 'good\nfair\nfair\npoor'
	};
	Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log('Band info sent to Pebble successfully.');
			getWeather(); // Once we finish our bands fetch, grab weather
		},
		function(e) {
			console.log('Error sending band info to Pebble. Error:' + e.message);
			getWeather(); // Once we finish bands, get weather.
		}
	);
}

/// Weather fetching
function locationSuccess(pos) {
  // We will request the weather here
 // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log('Temperature is ' + temperature);
      var temperature_f = (temperature * 9/5) + 32
    
     var dictionary = {
       'KEY_TEMPERATURE_C': temperature,
       'KEY_TEMPERATURE_F': temperature_f
     };

     // Send to Pebble
     Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble! Error: ' + e.message);
       }
     );
   });
}

function locationError(err) {
  console.log('Error requesting location!');
}

// Geolocate, then make web service call to fetch weather
function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather and band data
    getBands();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
      getBands();
    }                     
);

