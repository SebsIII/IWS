# IWS
The Instantaneous Weather Station (**IWS**) is a weather station that returns instantaneous weather data once asked.
It works with a **nucleo F401RE MCU and a W5100 Ethernet Shield** with those sensors:

- *DHT11*: Temperature and Humidity
- *BMP280*: Temperature, Pressure and approx altitude
- *RAIN sensor*: Rain level

The whole station is then powered with a 230VAC-5VDC inverter built directly in the station.
The whole size of the facility is apporximately **15x15x5cm**.

## The output

The MCU along with the Ethernet Shield hosts a **HTTP server** which, when called, returns a **JSON string** containing all of the essential data of the station itself and the value of each sensor.
