/**
   This program is a simple "Hello World!" for two ESP8266 connected by ESPNow.
   The first ESP gets the server role, the second is the target.
   Both are initialized at the same time for the best results.
   First, the server ESP scans for possible targets (in that case, it is only one for the simplicity).
   (For more targets it is important to change the currentTarget)
   After adding the target to the target list, the server tries to connect to the target to get a "Hello World!" in respondtime.
   If the target answered, the message could be seen on the terminal of the server.

   @author Jero A
   @version 1.0
   @date 16.01.2020
*/


// for LED control
#include <Color.h>

#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
#include <espnow.h>
}

// unwanted modes should be commented out
#define DEBUG
#define TARGET
//#define SERVER

// Color Use Cases
#define POWER 0
#define INIT 1
#define CONN 2
#define RECV 3
#define WAIT 4
#define SEND 5
#define ERR 6
#define OUT 7


#define WIFI_CHANNEL 1
// server esp mac addresses for the targets
#ifdef TARGET
uint8_t SERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};
uint8_t SERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};
#endif


// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) HELLO_WORLD {
  char data[10];
} helloWorld;

// get a msg from other device
volatile boolean haveReading = false;

// set LED Pins for rot,green,blue
Color LED(15, 12, 13);

// define login data
#define BASE_SSID "LG_"
#define PSK  "TestTest!"

// target administration
#ifdef SERVER
uint8_t targetsFound = 0;
// 40 macs possible to connect to the server
uint8_t targetMacs[40][6];
uint8_t currentTarget = 0;
#endif


//---------------------------------------------------------------------------------------------

void setup() {

  changeGPIOstatus(INIT);


  Serial.begin(74880);
  Serial.println("===========================================================");
  Serial.print("Test ESP Now ");

#ifdef DEBUG
  Serial.println(": Debugmode on");
  Serial.println("===========================================================");
  Serial.println('\n');

#ifdef SERVER
  Serial.println("===========================================================");
  Serial.println("Server");
#endif

#ifdef TARGET
  Serial.println("===========================================================");
  Serial.println("Target");
#endif

  Serial.println("-----------------------------------------------------------");
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());
#endif

  Serial.println("===========================================================");
  Serial.println('\n');

  initEspNow();

#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("Setup done");
#ifdef TARGET
  Serial.println("-----------------------------------------------------------");
  Serial.println("loop is starting");
#endif
  Serial.println("===========================================================");
  Serial.println('\n');
#endif
}

//------------------------------------------------------------------------------------------------------------------------------

void loop() {
#ifdef SERVER
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("loop is starting");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif

  // initialize all vars for the server
  uint8_t state = 0;

  // counter var
  uint8_t i = 0;

  // timer var
  long sendTime = 0;

  // timer var
  long startTime = 0;

  // message because we just read two chars out of the transfer array, but want to have a complet message
  String msg = "Hello World!";

  // timer for timeouts after this time (after 60 sec.)
  uint16_t respondTime = 60 * 1000;

  // transfer array for data
  uint8_t bs[sizeof(helloWorld)];

  // copy all data from bs to helloWorld structure
  memcpy(bs, &helloWorld, sizeof(helloWorld));

  while (1) {
    // Need some delay for watchdog feeding in loop
    delay(1);
    switch (state) {
      // no one is connected -> search for targets
      case 0:
        targetsFound = 0;
        scanForTargets();
        state++;
        break;

      // shows how many targets have been found
      case 1:
        if (targetsFound) {

          state++;
          // LED blink
          for (i = 0; i < targetsFound; i++) {
            changeGPIOstatus(CONN);
            delay(1000);
            changeGPIOstatus(OUT);
            delay(2000);
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.print("Server found Target ");
            // i + 1 -> just for better reading, first target else would be target 0
            Serial.print(i + 1);
            Serial.print(" of ");
            Serial.print(targetsFound);
            Serial.println(" Target/s.");
            Serial.println("===========================================================");
            Serial.println('\n');

#endif
          }
          changeGPIOstatus(SEND);
          // start time for the sending process
          startTime = millis();
          // no interesting message received
          haveReading = false;
        }
        // if no targets were found restart searching
        else {
          delay(100);
          state = 0;
        }
        break;

      // selects the first target of the target list and transmits the response time
      // the target knows what to do
      case 2:
        /*
            if you want to get answers from different targets: let currentTarget count up for every correct answer
            or = random(targetsFound); for random target choices
        */
        currentTarget = 0;
        // current time get splitt up in bs0 and bs1
        bs[0] = respondTime >> 8;
        bs[1] = respondTime & 0xFF;

#ifdef DEBUG
        Serial.println("===========================================================");
        Serial.print("Connected to Target ");
        Serial.print(currentTarget);
        Serial.print(" (MAC: ");
        for (i = 0; i < 6; i++) {
          Serial.print(targetMacs[currentTarget][i], HEX);
          if (i < 5) {
            Serial.print(":");
          } else {
            Serial.print(")");
          }
        }
        Serial.print(" for ");
        Serial.print(respondTime);
        Serial.println(" ms.");
        Serial.println("===========================================================");
        Serial.println('\n');
#endif

        // sends bs to the selected target
        esp_now_send(targetMacs[currentTarget], bs, sizeof(helloWorld));

        // start sending timer
        sendTime = millis();
        state++;
        changeGPIOstatus(WAIT);
        break;

      // analyses the answer of the target
      case 3:
        //if gets an answer
        if (haveReading) {
          changeGPIOstatus(RECV);
          // reset the message notification
          haveReading = false;

          //if target responds right
          if (helloWorld.data[0] == 'H' && helloWorld.data[1] == 'W') {
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.print("Target send : ");
            Serial.println(msg);
            Serial.println("===========================================================");
            Serial.println('\n');
#endif

            //target respond timeout
          } else if (helloWorld.data[0] == 2) {
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.println("Target reported a timeout");
            Serial.println("===========================================================");
            Serial.println('\n');

#endif
            // no correct message
            changeGPIOstatus(ERR);
          }

          //reaches the end
          state = 4;
        }

        //timeout timer for no response
        else if ((millis() - sendTime) > respondTime + 100) {
#ifdef DEBUG
          Serial.println("===========================================================");
          Serial.println("Target communication timeout");
          Serial.println("No answer from the Target");
          Serial.println("===========================================================");
          Serial.println('\n');
#endif

          changeGPIOstatus(ERR);
          delay(10000);

          //reaches the end
          state = 4;

        }
        Serial.flush();
        break;

      case 4:
        delay(2000);

        // try to connect with first target again
        state = 2;

        break;
    }
  }
#endif //end -  Server loop

  //____________________________________________________________________________________________________________________________

#ifdef TARGET

  uint16_t respondTime;
  /*
     if you want to delay the response, increase the wait time but keep an eye on the timeout
  */
  uint16_t waitTime = 10000;

  long startTime = 0;
  
  // marks the end of the intern loop
  boolean endFlag = false;

  // data array
  uint8_t bs[sizeof(helloWorld)];

  changeGPIOstatus(RECV);

  // if the target gets a message from the server
  if (haveReading) {
    // reset the message notification
    haveReading = false;

    changeGPIOstatus(WAIT);

    //time for the response
    respondTime = (helloWorld.data[0] << 8) | helloWorld.data[1];
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.print("activated for ");
    Serial.print(waitTime);
    Serial.println(" ms");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif
    // start time for the endFlag loop
    startTime = millis();

    while (endFlag == false) {
      // wathcdog
      delay(1);

      //if needs to long to answer -> timeout
      if (millis() - startTime > respondTime) {

        // message for the server
        bs[0] = 2;
        // stop endFlag loop
        endFlag = true;
#ifdef DEBUG
        Serial.println("===========================================================");
        Serial.println("Target timeout");
        Serial.println("===========================================================");
        Serial.println('\n');
#endif
      }

      //send the message after waitTime
      if (millis() - startTime > waitTime) {
        bs[0] = 'H';
        bs[1] = 'W';
        endFlag = true;
        Serial.println("===========================================================");
        Serial.println("Sending : Hello World!");
        Serial.println("===========================================================");
        Serial.println('\n');

      }
    }
    changeGPIOstatus(SEND);

    //send the message
    esp_now_send(SERVER_ap_mac, bs, sizeof(helloWorld));

    // wait until next loop start
    delay(10000);
  }

#endif
}

//------------------------------------------------------------------------------------------------------------------------------
// additional functions --------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------


void initEspNow() {
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("init methode");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif

  // get ssid of the device
  String ssid = String(BASE_SSID) + WiFi.softAPmacAddress().substring(9);
  WiFi.softAP(ssid.c_str(), PSK, WIFI_CHANNEL, 0, 0);
  WiFi.disconnect();

  // if initialization fails
  if (esp_now_init() != 0) {
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.println(" ESP_Now init failed");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif
    changeGPIOstatus(ERR);
    delay(3000);
    ESP.restart();
  }

  //choose the role that to be running with
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

#ifdef TARGET
  esp_now_add_peer(SERVER_ap_mac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(SERVER_sta_mac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.print("Add Server peer : ");
  Serial.print(" (MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(SERVER_ap_mac[i], HEX);
    if (i < 5) {
      Serial.print(":");
    } else {
      Serial.println(")");
    }
  }
  Serial.println("===========================================================");
  Serial.println('\n');
#endif
#endif

  // triggers everytime something is received by a device
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
    // copy the data of the transfer array to data
    memcpy(&helloWorld, data, sizeof(helloWorld));

    // Server handling for an incoming message
#ifdef SERVER
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.println("I got something O_O !!!");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif
    // check if the message comes from the right device
    if ((mac[0] == targetMacs[currentTarget][0])
        && (mac[1] == targetMacs[currentTarget][1])
        && (mac[2] == targetMacs[currentTarget][2])
        && (mac[3] == targetMacs[currentTarget][3])
        && (mac[4] == targetMacs[currentTarget][4])
        && (mac[5] == targetMacs[currentTarget][5])) {
      haveReading = true;
    }
#endif

    // What to do when a message comes from the server
#ifdef TARGET
    haveReading = true;
#endif

#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.println("end of receiving interrupt");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif
  });
}


//------------------------------------------------------------------------------------------------------------------------------


void scanForTargets() {             // only used by the server device

  int8_t scanResults = WiFi.scanNetworks();
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("Scan for targets");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif

  if (scanResults == 0) {
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.println("No WiFi devices in AP Mode found");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif

    changeGPIOstatus(ERR);
  }

  else {
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.print("Found ");
    Serial.print(scanResults);
    Serial.println(" potential Wifi devices ");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif

    for (int i = 0; i < scanResults; ++i) {


      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      // Save SSID, RSSI and BSSID for each device found that begins with "LG_"
      if (SSID.indexOf("LG_") == 0) {

        int mac[6];
        // check the BSSID and import it into the targetMacs list
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c",
                         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            targetMacs[targetsFound][ii] = (uint8_t) mac[ii];
          }
        }
        // increase the target counter
        targetsFound++;

#ifdef DEBUG
        Serial.println("===========================================================");
        Serial.print("Target ");
        // i + 1 -> just for better reading, first target else would be target 0
        Serial.print(i + 1);
        Serial.print(" of ");
        Serial.println(targetsFound);
        Serial.println("-----------------------------------------------------------");
        Serial.print("SSID of ");
        // i + 1 -> just for better reading, first target else would be target 0
        Serial.print(i + 1);
        Serial.print(" : ");
        Serial.println(SSID);
        Serial.println("-----------------------------------------------------------");
        Serial.print("RSSI of ");
        // i + 1 -> just for better reading, first target else would be target 0
        Serial.print(i + 1);
        Serial.print(" : ");
        Serial.println(RSSI);
        Serial.println("-----------------------------------------------------------");
        Serial.print("BSSIDstr of ");
        // i + 1 -> just for better reading, first target else would be target 0
        Serial.print(i + 1);
        Serial.print(" : ");
        Serial.println(BSSIDstr);
        Serial.println("===========================================================");
        Serial.println('\n');
#endif
      }
    }
  }
  // clean up ram
  WiFi.scanDelete();
}


//------------------------------------------------------------------------------------------------------------------------------


void changeGPIOstatus(uint8_t state) {
  switch (state) {
    // only power is given - default start val
    case 0: LED.yellow();   break;
    // initialisation process is running
    case 1: LED.blue();     break;
    // connection successful
    case 2: LED.white();    break;
    // data received from master/slave
    case 3: LED.green();    break;
    // hit me/wait for hit
    case 4: LED.cyan(); break;
    // got some data to send
    case 5: LED.violette(); break;
    // unexpected case
    default:
    // Error
    case 6: LED.red();      break;
    // turn LED off
    case 7: LED.black();    break;
  }
}
//------------------------------------------------------------------------------------------------------------------------------
