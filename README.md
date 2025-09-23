# IWS
The Instantaneous Weather Station (**IWS**) is a weather station that returns instantaneous weather data once asked.
It works with a **nucleo F401RE MCU and a W5100 Ethernet Shield** with those sensors:

- ~~*DHT11*: Temperature and Humidity (C°, %)~~ no longer working
- *BMP280*: Temperature, Pressure and approx altitude (C°, Pa, m)
- *RAIN sensor*: Rain level (1023 false, 0 true)

The whole station is then **powered with a 230VAC-5VDC inverter** built directly in the station.
The whole size of the facility is apporximately **15x15x5cm**.

## The goal

The main goal of the IWS is to **return raw weather data to each local system that needs them**, not to be a weather station only.
Because of that the output of the IWS is in JSON format, which is the best for sending and receiving data like IWS' Sensors.

The IWS data can be regulary sent to a local server wich could save and analyse them to get weather forsecast.

## Power consuption

The IWS has a power consumption of **220mA at 5VDC**  *1W~*, but since it is powered with 230VAC, the real current is **4,5 mA**.

In ideal conditions the IWS will reach the 1kW of power consumed in *1000 hours* or nearly **6 weeks**. 

## The output

The MCU along with the Ethernet Shield hosts a **HTTP server** which, when called, returns a **JSON string** containing all of the essential data of the station itself and the value of each sensor.

![JSON-HTTP](https://github.com/SebsIII/IWS/blob/43692908104991b1652bcf4ea6fd69c47edac72a/Gallery/JSON-HTTP.png)

Each sensor value refers to the **standard SI units**.

## The link

The HTTP server of the IWS can be found at:

```
192.168.1.200
```
