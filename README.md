### Google Calendar Events LED reminder 



I started this project because I wanted a visual reminder of simple tasks, like taking out the bins 
or cleaning the fridge. 

![Google Calendar LEDs](https://github.com/Niuslar/mqtt_calendar/blob/master/working_calendar.jpg =200x300)

The calendar consists of 35 RGB LEDs connected to an ESP32 which receives data via MQTT from a Raspberry Pi. The calendar receives data like date, calendar colour ID and other relevant information. This way, the events can be colour coded by creating different calendars with different colour IDs. 

I chose this approach because I wanted to practice my Python skills (with the R.Pi) and because I already had an MQTT broker running for another project. 

## First: Connect Raspberry Pi to Google Calendars 

The first step was to connect Google Calendars to the Raspberry Pi so it could read the data before publishing it via MQTT. The exact steps to do this can be found in [this link](https://developers.google.com/workspace/guides/getstarted-overview). As a base for the Python Script I used their ["quickstart"](https://developers.google.com/calendar/api/quickstart/python) script. It is a simple script that includes some of the APIs necessary to obtain the data. 

## Second: Publish the data via MQTT 

As I mentioned, I already had an MQTT broker running on the Raspberry Pi. I recommend looking at tutorials online to see how to install and use MQTT. The code that publishes the data to the "/events" topic can be found in the "mqtt_read_publish.py" file. 

## Third: Reading the data with the ESP32 

There are many ways to program an ESP32. In my case I used the official ESP-IDF, however, options like Platform.io or using the Arduino IDE can be much easier if you want to use other libraries. 
In my project [ESP32-MQTT](https://github.com/Niuslar/MQTT) I learnt how to use WiFi and MQTT with the ESP32 so I won't explain it here.
The files mqtt.c and mqtt.h in the "include" folder handle the different "events" (connected to broker, subscribed to topic, message received, etc.). The MQTT Client configuration is in the main file MQTT_CAL.c

## Fourth: Sending the data to the LEDs

Because I did this as a learning exercise, I chose to write my own library to control the LEDs. I used the WS2812B RGB LEDs, which can be individually addressed. These LEDs are very time sensitive, so I used the RMT peripheral to send the right signal according to the time constraints specified in the datasheet. The code for this can be found in the ws2812b.c and ws2812b.h files. 

## Fifth: Some problems I encountered 

One of the main problems I had was some random LEDs turning on/off for a fraction of a second. The problem ended up being the WiFi and MQTT interfering with the LEDs signal timing. To fix this, I created a task pinned to core 1 to send the LEDs signal and using the "menuconfig" included with the ESP-IDF I changed the core affinity for MQTT tasks to core 0 and pinned the WiFi tasks to the same core. This way the LEDs signal was not interrupted by the WiFi or MQTT interrupts. 

