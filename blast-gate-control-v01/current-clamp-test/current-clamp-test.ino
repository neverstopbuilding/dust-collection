// based on work done at: https://olimex.wordpress.com/2015/09/29/energy-monitoring-with-arduino-and-current-clamp-sensor/
// and: https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino?redirected=true
//
//double voltage = 0;
//
//void setup() {
//  Serial.begin(9600);
//  pinMode(A0, INPUT);
//}
//
//void loop() {
//  // put your main code here, to run repeatedly:
//
//  Serial.println(voltage);
//  voltage = analogRead(A0);
//  delay(1000);
//}


// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
double Irms = 0;
int counter = 0;
bool state = false;
bool priorState = false;

void setup()
{
  Serial.begin(9600);

  emon1.current(0, 111.1);             // Current: input pin, calibration.
}

void loop()
{
  Irms = emon1.calcIrms(2960);
  priorState = state;
  Serial.print(counter);
  Serial.print(" ");
  if (Irms > 0.2) {

    counter ++;
  } else {
    state = false;
    counter --;
  }
  if (counter > 5) {
    Serial.println("On for 5 cycles");
    state = true;
    counter = 5;
  }

  if (counter < 0) {
    Serial.println("OFF for 5 cycles");
    state = false;
    counter = 0;
  }

}
