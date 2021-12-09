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
int sensorValue;
double Irms = 0;
double voltage = 0;
double lastValue = 0;

void setup()
{
  Serial.begin(9600);
  emon1.current(0, 50);             // Current: input pin, calibration.
}

void loop()


{
  sensorValue  = analogRead(A0);  // read the input pin
float voltage= sensorValue * (5.0 / 1023.0);
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  double delta = Irms - lastValue;
  lastValue = Irms;
  Serial.print(voltage);

  Serial.print(" ");
  Serial.print(Irms);
  Serial.print(" ");
  Serial.print(delta);
  Serial.println("");




}
