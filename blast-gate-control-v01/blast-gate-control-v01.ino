// Based on the code: https://scottydwalsh.wordpress.com/automatic-dust-collection/
// Also: https://github.com/iliketomakestuff/iltms_automated_dust_collection/blob/master/DustCollectionAutomation_v2.ino
// based on work done at: https://olimex.wordpress.com/2015/09/29/energy-monitoring-with-arduino-and-current-clamp-sensor/
// and: https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino?redirected=true

#include "EmonLib.h"

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

    void hardwareTurnOn() {
      digitalWrite(startPin, 0);
      delay(1000);
      digitalWrite(startPin, 1);
    }

    void hardwareTurnOff() {
      digitalWrite(stopPin, LOW);
      delay(1000);
      digitalWrite(stopPin, HIGH);
    }

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

    bool isOff() {
      return not state;
    }

    void turnOn() {
      spinDownSeconds = 0;
      if (not isOn()) {
        writeToLog("Dust Collector Turned On");
        state = true;

//        hardwareTurnOn();

      }
    }
    bool isSpinningDown() {
      return spinDownSeconds > 0;
    }


    void turnOff(int spinDownSeconds) {
      this->spinDownSeconds = spinDownSeconds;
      writeToLog("Spindown Started");
    }


    void decrmentSpinDown() {
      Serial.print(spinDownSeconds);
      spinDownSeconds -= 1;
      if (spinDownSeconds == 0) {
        writeToLog("Dust Collector Turned Off");
        state = false;
//        hardwareTurnOff();
      }
      delay(1000);
    }
};

class blastGate {
    String gateName;
    int pin;
    bool state = false;

    void hardwareOpenGate() {
      digitalWrite(pin, LOW);
      delay(250);
    }

    void hardwareCloseGate() {
      digitalWrite(pin, HIGH);
      delay(250);
    }

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
        state = true;
//        hardwareOpenGate();
      }
    }

    void closeGate() {
      if (isOpen()) {
        writeToLog(gateName + " Blast Gate Closed");
//        hardwareCloseGate();
        state = false;
      }
    }
};

class machine {
    EnergyMonitor currentSensor;
    blastGate &primaryBlastGate;
    dustCollector &primaryDustCollector;



    double lastIrms = 0;
    double Irms = 0;
    double delta = 0;
    String machineName;
    int sensorPin;
    int spinDownSeconds;
    bool priorState = false;
    bool state = false;

  public:

    machine(String machineName, int sensorPin, int spinDownSeconds, blastGate &attachBlastGate, dustCollector &attachDustCollector):
      primaryBlastGate(attachBlastGate),
      primaryDustCollector(attachDustCollector)
    {
      this->machineName = machineName;
      this->sensorPin = sensorPin;
      this->spinDownSeconds = spinDownSeconds;
    }

    void setup() {
      currentSensor.current(0, 120);
      writeToLog(machineName + " initialized, sensor on pin " + sensorPin);
      setupDelay();
    }

    void checkUsage() {
      double Irms = currentSensor.calcIrms(1480);
      if (primaryDustCollector.isOff()) {
        primaryBlastGate.closeGate();
      }

            Serial.print(" ");
            Serial.print(Irms);
            Serial.print(" ");
            Serial.print(delta);
            Serial.print(machineName + " = ");
            Serial.print(state);
            Serial.println("");


      if (delta >= .1 && Irms > 0.32) {
        //        Serial.println("On state detected");
        state = true;
        if (state != priorState) {
          writeToLog(machineName + " is on.");
          primaryBlastGate.openGate();
          primaryDustCollector.turnOn();
          priorState = state;
        }
      }

      if (delta <= -.1) {
        //        Serial.println("off state detected");
        state = false;
        if (state != priorState) {
          writeToLog(machineName + " is off.");
          //          primaryBlastGate.closeGate();
          primaryDustCollector.turnOff(spinDownSeconds);
          priorState = state;
        }
      }
      priorState = state;
      delta = Irms - lastIrms;
      lastIrms = Irms;

    }


};

dustCollector mainDustCollector(7, 6);

blastGate planerGate("Planer", 5);
//blastGate bandSawGate("Band Saw", 12);
//blastGate jointerGate("Jointer", 11);
//blastGate tableSawGate("Table Saw", 10);
//blastGate shaperGate("Shaper", 9);
//blastGate floorSweepGate("Floor Sweep", 8);

machine planer("Planer", 0, 2, planerGate, mainDustCollector);
//machine bandSaw("Band Saw", 1, 5, bandSawGate, mainDustCollector);
//machine jointer("Jointer", 2, 10, jointerGate, mainDustCollector);
//machine tableSaw("Table Saw", 3, 5, tableSawGate, mainDustCollector);
//machine shaper("Shaper", 4, 5, shaperGate, mainDustCollector);
//machine floorSweep("Floor Sweep", 5, 5, floorSweepGate, mainDustCollector);

void setup() {
  delay(1000);
  Serial.begin(9600);


  mainDustCollector.setup();

  planerGate.setup();
//    bandSawGate.setup();
  //  jointerGate.setup();
  //  tableSawGate.setup();
  //  shaperGate.setup();
  //  floorSweepGate.setup();

  planer.setup();
//    bandSaw.setup();
  //  jointer.setup();
  //  tableSaw.setup();
  //  shaper.setup();
  //  floorSweep.setup();

}

void loop() {
  //  Serial.print(mainDustCollector.isSpinningDown());
  if (mainDustCollector.isSpinningDown()) {
    mainDustCollector.decrmentSpinDown();
  }

  planer.checkUsage();
//    bandSaw.checkUsage();
  //  jointer.checkUsage();
  //  tableSaw.checkUsage();
  //  shaper.checkUsage();
  //  floorSweep.checkUsage();



  //  mainDustCollector.turnOn();
  //  delay(3000);
  //  mainDustCollector.turnOff(0);
  //  delay(3000);


  // put your main code here, to run repeatedly:

  // for each tool
  // check if it is on
  // update gate states accoridingly
  // if ia tool is on turn on the dust collector
  // if no tool is on turn off the dust collector


}
