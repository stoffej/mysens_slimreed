/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * DESCRIPTION
 *
 * Simple binary switch example 
 * Connect button or door/window reed switch between 
 * digitial I/O pin 3 (BUTTON_PIN below) and GND.
 * http://www.mysensors.org/build/binary
 */


// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
#define MY_NODE_ID 13


#include <MySensor.h>
#include <SPI.h>
#include <Vcc.h>

#define SW_CHILD_ID 5
#define SW_PIN 3
#define BATTERY_REPORT_DAY 2   // Desired heartbeat interval when inactive. Maximum heartbeat/report interval is equal to this due to the dayCounter.
#define BATTERY_REPORT_BY_IRT_CYCLE 10  // Adjust this according to usage frequency.
#define ONE_DAY_SLEEP_TIME 86400000
#define VCC_MIN 1.9
#define VCC_MAX 3.3

int dayCounter = 0;
int irtCounter = 0;
uint8_t value;
uint8_t sentValue=2;
bool interruptReturn=false;

Vcc vcc;
MySensor gw;
MyMessage msg(SW_CHILD_ID, V_TRIPPED);

void setup()
{
  delay(100); // to settle power for radio

  pinMode(SW_PIN, INPUT);
  digitalWrite(SW_PIN, LOW);    // Disable internal pull-ups
}

void presentation(){
  sendSketchInfo("SlimReed", "0.1 2016-08-30");
  present(SW_CHILD_ID, S_DOOR);
}

void loop()
{
  if (!interruptReturn) { // Woke up by timer (or first run)
    dayCounter++;
    if (dayCounter >= BATTERY_REPORT_DAY) {
          dayCounter = 0;
          sendBatteryReport();
    }
  }
  else {    // Woke up by pin change
      irtCounter++;
      gw.sleep(50);       // Short delay to allow switch to properly settle
      value = digitalRead(SW_PIN);
      if (value != sentValue) {
         gw.send(msg.set(value==HIGH ? 1 : 0));
         sentValue = value;
      }
      if (irtCounter>=BATTERY_REPORT_BY_IRT_CYCLE) {
        irtCounter=0;
        sendBatteryReport();
      }
  }

  // Sleep until something happens with the sensor,   or one sleep_time has passed since last awake.
  interruptReturn = gw.sleep(SW_PIN-2, CHANGE, ONE_DAY_SLEEP_TIME);

}

void sendBatteryReport() {
          float p = vcc.Read_Perc(VCC_MIN, VCC_MAX, true);
          int batteryPcnt = static_cast<int>(p);
          gw.sendBatteryLevel(batteryPcnt);
}
