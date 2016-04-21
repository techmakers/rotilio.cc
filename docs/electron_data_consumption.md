# Electron data consumption

Here follow a synthesis about Electron cellular data consumption

### Just to be live
Ping: 318 bytes per hour ==> 228.960 per month (22% of first MB)

### Every publish and read variable
100+ bytes over the size of data (event/variable name and data)

## How to limit data consumption
- Use USB firmware update instead OTA firmware update
- Publish only if a value is really and significantly changed (example: send temperature only if it varies more than .5°C)
- Count the publish per period and limit the publish frequency if possible
- Don't poll for variable if is not necessary
- Reduce variable and event name length to the minimum
- Accumulate publish and variables in one time (string concat):
	- maximum string length for variable value is 622 bytes
	- maximum string length for publish data is 255 bytes
- Use System.sleep() to reduce data exchange (and battery life) [https://docs.particle.io/reference/firmware/electron/#sleep-sleep-](https://docs.particle.io/reference/firmware/electron/#sleep-sleep-)
- If possible write firmware with this two guide lines:
	- work even if in offline mode (ex: if data limit reached, Electron goes offline)
	- work with local algorithm if you can, to avoid dialog with the cloud
