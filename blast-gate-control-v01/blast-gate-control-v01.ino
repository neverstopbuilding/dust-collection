// Based on the code: https://scottydwalsh.wordpress.com/automatic-dust-collection/
// Also: https://github.com/iliketomakestuff/iltms_automated_dust_collection/blob/master/DustCollectionAutomation_v2.ino

void writeToLog(String eventMessage) {
  Serial.println(eventMessage);
}

void setupDelay() {
  delay(250);
}

class dustCollector {
    //Assumes that relays are used to automate the pressing of the start or stop motor starter circuits
    //Start is typically wired normally open
    //Stop is typically wired normally closed
    int startPin;
    int stopPin;
    bool state = false;
    int spinDownSeconds = 0;


  public:
    dustCollector(int startPin, int stopPin) {
      this->startPin = startPin;
      this->stopPin = stopPin;
    }

    void setup() {
      pinMode(startPin, OUTPUT);
      pinMode(stopPin, OUTPUT);
      digitalWrite(startPin, 1);
      digitalWrite(stopPin, 1);
      writeToLog("Dust collector initialized.");
      setupDelay();
    }

    bool isOn() {
      return state;
    }

    void turnOn() {
      if (not this->isOn()) {
        writeToLog("Dust Collector Turned On");
        digitalWrite(startPin, 0);
        state = true;
        delay(2000);
        digitalWrite(startPin, 1);
      }
    }

    void turnOff(int spinDownSeconds) {
      this->spinDownSeconds = spinDownSeconds;
      if (not isSpinningDown()) {
        writeToLog("Dust Collector Turned Off");
        digitalWrite(stopPin, LOW);
        state = false;
        delay(2000);
        digitalWrite(stopPin, HIGH);
      }
    }

    bool isSpinningDown() {
      spinDownSeconds > 0 && isOn();
    }

    void decrmentSpinDown() {
      delay(1000);
      spinDownSeconds -= 1;
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
      setupDelay();

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

class machine {
    String machineName;
    int sensorPin;
    int spinDownSeconds;
    bool priorState = false;
    bool state = false;
    blastGate &primaryBlastGate;
    dustCollector &primaryDustCollector;

  public:

    machine(String machineName, int sensorPin, int spinDownSeconds, blastGate &primaryBlastGate, dustCollector &primaryDustCollector) {
      this->machineName = machineName;
      this->sensorPin = sensorPin;
      this->spinDownSeconds = spinDownSeconds;
    }

    void setup() {
      pinMode(sensorPin, INPUT);
      writeToLog(machineName + " initialized, sensor on pin " + sensorPin);
      setupDelay();
    }

    void checkUsage() {
      if (isOn()) {
        state = true;
        if (state != priorState) {
          writeToLog(machineName + " is on.");
          primaryBlastGate.openGate();
          primaryDustCollector.turnOn();
          priorState = state;
        }
      }

      if (isOff()){
        state = false;
        if (state != priorState){
          writeToLog(machineName + " is off.");
          primaryBlastGate.closeGate();
          primaryDustCollector.turnOff(spinDownSeconds);
          priorState = state;
        }
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

machine planer("Planer", A0, 10, planerGate, mainDustCollector);
machine bandSaw("Band Saw", A1, 5 , bandSawGateGate, mainDustCollector);
machine jointer("Jointer", A2, 10 , jointerGate, mainDustCollector);
machine tableSaw("Table Saw", A3, 5 , tableSawGate, mainDustCollector);
machine shaper("Shaper", A4, 5 , shaperGate, mainDustCollector);

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

  planer.setup();
  bandSaw.setup();
  jointer.setup();
  tableSaw.setup();
  shaper.setup();
  floorSweep.setup();

}

void loop() {
  if (mainDustCollector.isSpinningDown()) {
    mainDustCollector.decrmentSpinDown();
  }

  planer.checkUsage();
  bandSaw.checkUsage();
  jointer.checkUsage();
  tableSaw.checkUsage();
  shaper.checkUsage();
  floorSweep.checkUsage();



  mainDustCollector.turnOn();
  delay(3000);
  mainDustCollector.turnOff();
  delay(3000);


  // put your main code here, to run repeatedly:

  // for each tool
  // check if it is on
  // update gate states accoridingly
  // if ia tool is on turn on the dust collector
  // if no tool is on turn off the dust collector


}
