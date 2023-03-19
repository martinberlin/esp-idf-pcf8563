# esp-idf-pcf8563
PCF8563 RTC Driver for esp-idf

# Installation
```
git clone https://github.com/martinberlin/esp-idf-pcf8563.git
cd esp-idf-pcf8563
idf.py menuconfig
idf.py flash
```

## Wiring  

Proven to work configurations

|PCF8563||ESP32|||ESP32-C3|
|:-:|:-:|:-:|:-:|
|SCL|--|IO22(*1)|IO4|
|SDA|--|IO21(*1)|IO5|
|GND|--|GND|GND|
|VCC|--|3V3|3V3|

(*1) You can change using menuconfig.   

## Set clock / Get clock mode

In order to see a demo implementation that sets the time reading an internet time source (NTP) please head over to the [Bistable smart-switch repository](https://github.com/martinberlin/bistable-smart-switch/tree/master/main/switch/rtc)
_
There in the folder **/main/switch/rtc** there is a pcf8563-switch.cpp that implements this using set_time() and has the necessary functions that you can use and adapt to your project.


## Comparison with other RTCs

This module has a time lag of some seconds per month so do a sync each 3 weeks or so if you make a clock to stay in the safe area.   
I recommend the DS3231 RTC for more precision since has a temperature compensated crystal.
