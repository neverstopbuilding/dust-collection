

int mVperAmp = 185; // use 100 for 20A Module and 66 for 30A Module
double ampThreshold = .20;

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

//button debouncing
int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(A0, INPUT);
}

void loop() {

  Serial.println("----------");
  checkForAmperageChange();

}
boolean checkForAmperageChange() {
  Voltage = getVPP(A0);
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = (VRMS * 1000) / mVperAmp;
//  Serial.print(tools[which] + ": ");
  Serial.print(AmpsRMS);
  Serial.println(" Amps RMS");
  if (AmpsRMS > ampThreshold) {
    return true;
  } else {
    return false;
  }
}

float getVPP(int sensor)
{
  float result;

  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 500) //sample for 1 Sec
  {
    readValue = analogRead(sensor);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the maximum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 5.0) / 1024.0;

  return result;
}
