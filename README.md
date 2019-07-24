# esp-idf-pcf8563
PCF8563 RTC Driver for esp-idf

```
git clone https://github.com/nopnop2002/esp-idf-pcf8563
cd esp-idf-pcf8563
make menuconfig
make flash
```

---

# Set Clock Mode   

This mode set RTC initial value using NTP time Server.   
you have to set these config value using menuconfig.   

![config-pcf8563-1](https://user-images.githubusercontent.com/6020549/61298760-c38da700-a819-11e9-858c-a1ef2f03cf18.jpg)

![config-pcf8563-2](https://user-images.githubusercontent.com/6020549/61298768-c7b9c480-a819-11e9-9d4a-c2fe71a92e83.jpg)

you have to set gpio & NTP Server using menuconfig.   

![config-pcf8563-11](https://user-images.githubusercontent.com/6020549/61298824-de601b80-a819-11e9-8ccd-7d6b03fc3259.jpg)

![config-pcf8563-12](https://user-images.githubusercontent.com/6020549/61298840-e3bd6600-a819-11e9-8bc2-1c36525bc2b6.jpg)

---

# Get Clock Mode   

This mode take out the time from a RTC clock.   
you have to change mode using menuconfig.   

![config-pcf8563-11](https://user-images.githubusercontent.com/6020549/61298824-de601b80-a819-11e9-8ccd-7d6b03fc3259.jpg)

![config-pcf8563-13](https://user-images.githubusercontent.com/6020549/61298918-0d768d00-a81a-11e9-996a-a2660af4f665.jpg)

![pcf8563-13](https://user-images.githubusercontent.com/6020549/61298923-10717d80-a81a-11e9-91c4-2136030eb2eb.jpg)

---

# Get the time difference of NTP and RTC   

This mode get time over NTP, and take out the time from a RTC clock.   
Calculate time difference of NTP and RTC.   
you have to change mode using menuconfig.   

![config-pcf8563-11](https://user-images.githubusercontent.com/6020549/61298824-de601b80-a819-11e9-8ccd-7d6b03fc3259.jpg)

![config-pcf8563-14](https://user-images.githubusercontent.com/6020549/61298984-35fe8700-a81a-11e9-80e0-7399fdbbff43.jpg)

![pcf8563-14](https://user-images.githubusercontent.com/6020549/61298986-38f97780-a81a-11e9-99ab-6c4954373735.jpg)

---

# Time difference of 1 week later.   

![pcf8563-1week](https://user-images.githubusercontent.com/6020549/61800860-c5361b00-ae68-11e9-8b7d-ed64529f25fe.jpg)
