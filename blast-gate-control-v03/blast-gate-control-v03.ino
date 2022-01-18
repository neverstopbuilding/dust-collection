#define CURRENT_TRIGGER 1
#define SWITCH_TRIGGER 0


//TODO: Check Start up delay on floor sweep cause of suction bounce


int mVperAmp = 20.14; // use 100 for 20A Module and 66 for 30A Module


double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;


const int DEBUG_NUMBER_OF_TOOLS = 2;
const int NUMBER_OF_TOOLS = 6;
String tools[NUMBER_OF_TOOLS] = {"Planer", "Bandsaw", "Jointer", "Table Saw", "Shaper", "Floor Sweep"};
double toolCurrentThreshold[NUMBER_OF_TOOLS] = {15,3,2.5,8,8,0};
int sensorPin[NUMBER_OF_TOOLS] = {A0, A1, A2, A3, A4, 2};
int toolState[NUMBER_OF_TOOLS] = {0, 0, 0, 0, 0, 0};
int lastToolState[NUMBER_OF_TOOLS] = {0, 0, 0, 0, 0, 0};
int toolTriggerType[NUMBER_OF_TOOLS] = {CURRENT_TRIGGER, CURRENT_TRIGGER, CURRENT_TRIGGER, CURRENT_TRIGGER, CURRENT_TRIGGER, SWITCH_TRIGGER};
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int buttonState;
int lastButtonState = HIGH;  // the previous reading from the input pin
const int NUMBER_OF_GATES = 6;
String gateNames[NUMBER_OF_GATES] = {"Sweep Leg", "Shaper", "Planer", "Table Saw", "Bandsaw", "Jointer"};
int gates[NUMBER_OF_TOOLS][NUMBER_OF_GATES] = {
  {0, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1},
  {0, 0, 0, 1, 0, 0},
  {1, 1, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0}
};
int gateState[NUMBER_OF_GATES] = {0, 0, 0, 0, 0, 0};

int gateSolenoidPin[NUMBER_OF_GATES] = {12, 11, 10, 9, 8, 7};


bool collectorIsOn = 0;
const int DUST_COLL_START_PIN = 6;
const int DUST_COLL_STOP_PIN = 5;
bool spinDownState = false;
unsigned long shutDownTime = 0;
bool startUpState = false;
unsigned long startUpTime = 0;
const int SPIN_DOWN_TIME = 6000;
const int DUST_COLLECTOR_START_DELAY = 2000;

int thisReading = 0;
int lastReading = 0;
unsigned long firstReadingTime = 0;
const int BUTTON_DEBOUNCE_DELAY = 100;

bool toolStarting = false;
unsigned long gateOpenTime =0;
const int GATE_OPEN_DELAY = 1000;


void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(DUST_COLL_START_PIN, OUTPUT);
  digitalWrite(DUST_COLL_START_PIN, HIGH);
  pinMode(DUST_COLL_STOP_PIN, OUTPUT);
  digitalWrite(DUST_COLL_STOP_PIN, HIGH);
  for (int i = 0; i < NUMBER_OF_GATES; i++ ) {
    pinMode(gateSolenoidPin[i], OUTPUT);
    digitalWrite(gateSolenoidPin[i], HIGH);
  }
  for (int i = 0; i < NUMBER_OF_TOOLS; i++ ) {
    pinMode(sensorPin[i], INPUT);
  }
}

void loop() {
  Serial.println("----------");
  int aToolIsOn = 0;
  for (int i = 0; i < NUMBER_OF_TOOLS; i++) {
    if (toolTriggerType[i] == CURRENT_TRIGGER) {
      toolState[i] = checkForAmperageChange(i);
    } else {

      thisReading = checkForStateChange(i);
      if (millis() - firstReadingTime >= BUTTON_DEBOUNCE_DELAY && thisReading == lastReading) {
        toolState[i] = thisReading;
      } else {
        lastReading = thisReading;
        firstReadingTime = millis();
      }
      toolState[i] = checkForStateChange(i);
    }
    aToolIsOn += toolState[i];
  }

  for (int j = 0; j < NUMBER_OF_GATES; j++) {
    int stateSum = 0;
    for (int i = 0; i < NUMBER_OF_TOOLS; i++) {
      stateSum += gates[i][j] && toolState[i];
      gateState[j] = stateSum > 0;
    }
  }
  if (aToolIsOn) {
    if (toolStarting) {
      if (millis() >= gateOpenTime) {
        //open gates before closing the others
        for (int i = 0; i < NUMBER_OF_GATES; i++) {
          if (gateState[i]) {
            openGate(i);
          }
        }
        //TODO: delay between opening and closing
        for (int i = 0; i < NUMBER_OF_GATES; i++) {
          if (!gateState[i]) {
            closeGate(i);
          }
        }
      }
      toolStarting = false;
    } else {
      toolStarting = true;
      gateOpenTime = millis() + GATE_OPEN_DELAY;
    }


    if (collectorIsOn) {
      spinDownState = false;
    } else {
      if (startUpState) {
        if (millis() >= startUpTime) {
          startDustCollector();
          startUpState = false;
        }
      } else {
        Serial.println("Starting...");
        startUpState = true;
        startUpTime = millis() + DUST_COLLECTOR_START_DELAY;
      }
    }
    setLastToolStatetoToolState();
  } else {
    if (collectorIsOn) {
      if (spinDownState) {
        if (millis() >= shutDownTime) {
          stopDustCollector();
          //Leaving whatever gates were last open-opened to avoid starving the collector
        } else {
          Serial.print(shutDownTime - millis());
        }
      } else {
        Serial.println("Spinning Down...");
        spinDownState = true;
        if (lastToolIsFloorSweep()) {
          //If the last tool to be "turned off" is the floor sweep, just shut the dc down so not to starve
          shutDownTime = millis();
        } else {
          shutDownTime = millis() + SPIN_DOWN_TIME;
        }
      }
    } else {
      startUpState = false;
      //do nothing tools are off and collector is off
    }
  }
  Serial.print("Tool State: { ");
  for (int i = 0; i < NUMBER_OF_TOOLS; i++) {
    Serial.print(toolState[i]);
    Serial.print(" ");
  }
  Serial.println("}");
  Serial.print("A Tool is On: ");
  Serial.println(aToolIsOn > 0);

  Serial.print("Gate State: { ");
  for (int i = 0; i < NUMBER_OF_GATES; i++) {
    Serial.print(gateState[i]);
    Serial.print(" ");
  }
  Serial.println("}");
}
boolean checkForAmperageChange(int which) {
  Voltage = getVPP(sensorPin[which]);
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = (VRMS * 1000) / mVperAmp;
//  Serial.print(tools[which] + ": ");
//  Serial.print(AmpsRMS);
//  Serial.print(" Amps RMS - State: ");
  if (AmpsRMS > toolCurrentThreshold[which]) {
    return 1;
  } else {
    return 0;
  }
}

boolean checkForStateChange(int which) {
  int reading = digitalRead(sensorPin[which]);
  //TODO: Is Debouncing Nessesary?
  buttonState = reading;
  Serial.print(tools[which] + ": ");
  Serial.println(buttonState);
  return buttonState;
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

void openGate(uint8_t gate) {
  // Serial.print("openGate: ");
  // Serial.println(gateNames[gate]);
  digitalWrite(gateSolenoidPin[gate], LOW);
}

void closeGate(uint8_t gate) {
  //Serial.print("closeGate: ");
  //Serial.println(gateNames[gate]);
  digitalWrite(gateSolenoidPin[gate], HIGH);
}

void startDustCollector() {
  Serial.println("startDustCollector");
  digitalWrite(DUST_COLL_START_PIN, LOW);
  delay(1000);
  digitalWrite(DUST_COLL_START_PIN, HIGH);
  collectorIsOn = true;
}

void stopDustCollector() {
  Serial.println("stopDustCollector");
  digitalWrite(DUST_COLL_STOP_PIN, LOW);
  delay(1000);
  digitalWrite(DUST_COLL_STOP_PIN, HIGH);
  collectorIsOn = false;
}

bool lastToolIsFloorSweep() {
  int checkState[NUMBER_OF_TOOLS] = {0, 0, 0, 0, 0, 1};
  for (int i = 0; i < NUMBER_OF_TOOLS; i++) {
    if (lastToolState[i] != checkState[i]) {
      return false;
    }
  }
  return true;
}

void setLastToolStatetoToolState() {
  for (int i = 0; i < NUMBER_OF_TOOLS; i++) {
    lastToolState[i] = toolState[i];
  }
}
