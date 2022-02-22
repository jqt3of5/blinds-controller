# Blinds Controller

In my house, I have remote controlled blinds covering windows that are difficult to access (they're about 15 feet up).
I had been thinking about buying a USB software defined radio reciever, and figured this would  be the perfect project to use as an excuse to finally buy one. 

The blinds operate on 433MHz, using OOK modulation. After playing around with the SDR, and recording the signal, I was able to reverse engineer the specific protocol used. You can see this protocol in blinds.cpp. 

This was designed to run on an esp32, and connect to home assistant. It uses the build in capacitive touch sensor pins as the buttons on the face of the enclosure.  
