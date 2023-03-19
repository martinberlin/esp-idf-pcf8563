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

|PCF8563||ESP32||ESP32-C3|
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

## Additional functions

**set_alarm** at 3PM ignore day number and week day

```C
  tm alarm; // C timestruct
  http://alarm.tm_hour = 15;
  http://alarm.tm_min = 0;
  http://alarm.tm_mday = -1;
  http://alarm.tm_wday = -1;
  pcf8563_set_alarm(&i2c, &alarm);
```

Sometimes you just need to execute something either for some time and stop, or repetitively each 10 minutes or so. PCF8563 has a nice and verz configurable timer function.


```C
// 8.8.1 Register Timer_control
enum {
    PCF8563_CLK_4096HZ,
    PCF8563_CLK_64HZ,     // Giving 30 per second
    PCF8563_CLK_1HZ,       // per Second
    PCF8563_CLK_1_div_60HZ // timer Minute
};
 // Every second         1 sec= 1 HZ , divider (If it would be two then will tick 2x per second)
  pcf8563_set_timer(&dev, PCF8563_CLK_1HZ, 1);
  pcf8563_enable_timer(&dev);
  
  // Clear flag after reading so it comes again next timer
  pcf8563_get_flags(&dev);
  
  // Every two minutes
  pcf8563_set_timer(&dev, PCF8563_CLK_1_div_60HZ, 2);
```


## Comparison with other RTCs

This module has a time lag of some seconds per month so do a sync each 3 weeks or so if you make a clock to stay in the safe area.   
I recommend the DS3231 RTC for more precision since has a temperature compensated crystal.
