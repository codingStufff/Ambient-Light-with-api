# Ambient-Light-with-api
This program enables you to fetch weather data from darksky using a wemos esp 8266 board and make a dmx lamp simulate the light of the outside. It does this by looking at cloud cover, and unix time in order to simulate sunset.  The code in this project is loosely borrowed from another project also using dark sky API, the git hub link is here https://github.com/RuneBarrett/IntelliLight.  

Hardware:
•	Wemos esp 8266 
•	Dmx shield
•	Jumper wire 
•	One or more dmx lamps + cables
![alt text](https://github.com/codingStufff/Ambient-Light-with-api/blob/master/IMG_1538%202.JPG = 200x200)  
Software:
•	Arduino IDE 1.8.5 newer versions may be compatible
Libraries:
•	ArduinoJson.h
•	ESP8266WiFi.h 
•	Sun.h
 
Small note to the sun library. We changed the methods rise and set, to sunrise and sunset. This was due to set being a keyword in the Arduino IDE already.

Getting started:
First off, in order to get access to the darksky API you need to make an account at https://darksky.net. After the creation, and you logged in, you should maneuver to the API tap. From there just copy paste the API – there is a “secret code” but its already in the actual API so copy pasting is fine.
In order to get the program working, you will need a Wi-Fi connection and of course, know the password. At the top of the program write in the name of the internet and the password.
 
If the program should work precise according to your individual position, the Sun library needs both longitude and latitude.
 
We also initialize how many ports we want to use regarding the dmx lamps.
 
The entire void loop function can be modified to your preferences. If you simply want to change the speed pr. Call to dark sky, you have to change period.
 
Remember that 1000 milliseconds = 1 second. Further it should be noted, that this rate of requests has been chosen in order to test the program, and dark sky only allows 1000 calls pr. Day. Everything exceeding this will require a payment. 

Happy illuminating.
