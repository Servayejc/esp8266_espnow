JC Servaye
Complete project details at https://github.com/Servayejc/esp_now_sender/

---------------- Powering ESP8266 with AA batteries ----------------
 
 In my case, I need to send pool temperature to the server each minute. For the pool, I will not use power supply for security reasons.

 To powering with batteries, we need to limit the current and time needed to run our application.
 
 - Using the minimum hardware possible.    
 - Using ESP-NOW and not WIFI because ESP-NOW transmit data without the need of maintaining permanant WiFi connection.
 - Setting the ESP in deep sleep mode for a maximum of time, waking up only for getting the data and transmit.

 I choosed ESP12F because in all other development bords, only the ESP8266 can be set in deepsleep mode, but USB to serial device stay powered. 
 I uses a pack of 4 AA batteries and a voltage regulator (MCP1702 3.3 volts: dropout = 0.75V, quescient current = 2 uA )
 To keep 3.3V on the ESP8266, the batteries voltage must be 3.3V + 0.75V = 4.05 Volts minimum.
 The total current in deep sleep is now 12 uA. 

 After powerup, the code need to make a pairing with the server, this takes less 2.5 seconds running time.

        Searching 1-wire bus
        End of search : 18ms
        Setup done
        Pairing request on channel 1
        Pairing request on channel 2
        Pairing request on channel 3
        Pairing request on channel 4
        Pairing request on channel 5
        Pairing done on channel 6 in 2333ms for 78:e3:6d:09:fc:89
        Send data of device 1
        I'm awake from 2366ms , but I'm going into deep sleep mode for 10 seconds

 After wakeup, the running time is less then 350ms

        Searching 1-wire bus
        End of search : 18ms
        Setup done
        Pairing request on channel 5
        Pairing done on channel 6 in 285ms for 78:e3:6d:09:fc:89
        Send data of device 1
        I'm awake from 318ms , but I'm going into deep sleep mode for 10 seconds
 
To perform a test in a reasonable time, I make tests with a sleep time of 10 seconds and, after 1 month, the battery voltage is about 4 volts.
So, if I need a transmission each minute, the 4 AA battery can do the job at least for 6 months (in Canada, we don't have 6 months of summer time!). 
  
