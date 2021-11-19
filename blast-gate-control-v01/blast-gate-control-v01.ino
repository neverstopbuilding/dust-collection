

// Based on the code: https://scottydwalsh.wordpress.com/automatic-dust-collection/
// Also: https://github.com/iliketomakestuff/iltms_automated_dust_collection/blob/master/DustCollectionAutomation_v2.ino

void writeToLog(String eventMessage) {
  Serial.println(eventMessage);
}

class dustCollector {
    //Assumes that relays are used to automate the pressing of the start or stop motor starter circuits
    //Start is typically wired normally open
    //Stop is typically wired normally closed
    int startPin;
    int stopPin;
    bool state = false;

  public:
    dustCollector(int startPin, int stopPin) {
      this->startPin = startPin;
      this->stopPin = stopPin;
    }

    void setup() {
      pinMode(startPin, OUTPUT);
      pinMode(stopPin, OUTPUT);
      digitalWrite(startPin, HIGH);
      digitalWrite(stopPin, HIGH);
      writeToLog("Dust collector initialized.");
      delay(500);
    }

    bool isOn() {
      return state;
    }

    void turnOn() {
      if (not isOn()) {
        writeToLog("Dust Collector Turned On");
        digitalWrite(startPin, LOW);
        state = true;
        delay(2000);
        digitalWrite(startPin, HIGH);
      }
    }

    void turnOff() {
      if (isOn()) {
        writeToLog("Dust Collector Turned Off");
        digitalWrite(stopPin, LOW);
        state = false;
        delay(2000);
        digitalWrite(stopPin, HIGH);
      }
    }
};

class blastGate {
    String gateName;
    int pin;
    bool state = false;

  public:

    blastGate(String gateName, int pin) {
      this->gateName = gateName;
      this->pin = pin;
    }

    void setup() {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);
      writeToLog(gateName + " Blast gate initialized on pin " + pin);
      delay(500);

    }

    bool isOpen() {
      return state;
    }

    void openGate() {
      if (not isOpen()) {
        writeToLog(gateName + " Blast Gate Opened");
        digitalWrite(pin, LOW);
        state = true;
        delay(1000);
      }
    }

    void closeGate() {
      if (isOpen()) {
        writeToLog(gateName + " Blast Gate Closed");
        digitalWrite(pin, HIGH);
        state = false;
        delay(1000);
      }
    }
};

dustCollector mainDustCollector(7, 6);

blastGate planerGate("Planer", 13);
blastGate bandSawGate("Band Saw", 12);
blastGate jointerGate("Jointer", 11);
blastGate tableSawGate("Table Saw", 10);
blastGate shaperGate("Shaper", 9);
blastGate floorSweepGate("Floor Sweep", 8);

void setup() {
  delay(1000);
  Serial.begin(9600);

  mainDustCollector.setup();

  planerGate.setup();
  bandSawGate.setup();
  jointerGate.setup();
  tableSawGate.setup();
  shaperGate.setup();
  floorSweepGate.setup();

}

void loop() {
  // put your main code here, to run repeatedly:

  // for each tool
  // check if it is on
  // update gate states accoridingly
  // if ia tool is on turn on the dust collector
  // if no tool is on turn off the dust collector

}
