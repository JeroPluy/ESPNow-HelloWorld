/**
 * A small program to get the AP mac and the STA mac from the ESP 
 *
 *  @author Jero A
 *  @version 1.0
 *  @date 22.01.2020
 */

// for LED control
#include <Color.h>

#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
#include <espnow.h>
}

// set LED Pins for rot,green,blue
Color LED(15, 12, 13);

void setup() {
  Serial.begin(74880);
  Serial.println("===========================================================");
  Serial.println("Gameserver");
  Serial.println("-----------------------------------------------------------");
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());
  Serial.println("===========================================================");
  Serial.println('\n');
}

void loop() {
  LED.green();

}
