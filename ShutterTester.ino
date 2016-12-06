#define STATE_READY 'R'
#define STATE_START_MEASURING 'M'
#define STATE_SHUTTER_OPEN 'O'
#define STATE_CALIBRATING 'C'


int hiLdr = 0;
int loLdr = 0;
int ldr = 0;
int threshold = 0;
String command;
char state = STATE_READY;
unsigned long shutterOpen = 0;
unsigned long shutterClose = 0;
unsigned long printed;
unsigned long highest;

void setup(){
  Serial.begin(9600);
  Serial.println("Ready");
  command = "";
  printed = millis();
  pinMode(A2, OUTPUT);
}

void setThreshold() {
  threshold = hiLdr - ((hiLdr - loLdr)/4);
}

void loop(){
  ldr = 1024 - analogRead(A0); 

  if(Serial.available()) {
    doCommands();
  }

  if(state == STATE_START_MEASURING) {
    if(ldr > highest) {
      highest = ldr;
    }
    if(ldr > threshold) { // shutter has opened
      shutterOpen = micros();
      state = STATE_SHUTTER_OPEN;   
      Serial.println("Open");
    }
  } else if(state == STATE_SHUTTER_OPEN) {
    if(ldr < threshold) { // shutter has closed again
      shutterClose = micros();
      state = STATE_READY;
      Serial.println("Annnd closed");
      String printState = "";
      float duration = shutterClose - shutterOpen;
      duration /= 1000; //milliseconds
      printState = printState + "Duration: "+ duration;

      if(duration > 500) { // longer than 1/2 second
          duration = duration / 1000;
          printState = printState + " " + duration + " seconds"; 
      } else { // shorter than a second
          duration = 1000 / duration;
          int rounded = (int) duration;
          printState = printState + " or ~ 1/"+rounded + " seconds"; 
      }
      Serial.println(printState);
    }
  } else if (state == STATE_CALIBRATING) {
    if(ldr > hiLdr) {
      hiLdr = ldr;
    }

    if(ldr < loLdr) {
      loLdr = ldr;
    }

    if(hiLdr - loLdr > 10) { //Sufficient difference, lets do it.
      setThreshold();
      state = STATE_READY;
    }
  }

  printState();
}

void printState() {
  unsigned long now = millis();
  if(state == STATE_READY && now > (printed+1000)) {
    printed = now;
    String out = "";
    out.concat("State: ");
    out.concat(state);
    out.concat(" ldr: ");
    out.concat(ldr);
    out.concat(" hi: ");
    out.concat(hiLdr);
    out.concat(" lo: ");
    out.concat(loLdr);
    out.concat(" threshold: ");
    out.concat(threshold);

    Serial.println(out);
  }
}

void laser(bool on) {
  if(on) {
    Serial.println("Laser On");  
    digitalWrite(A2, HIGH);
  } else {
    Serial.println("Laser Off");  
    digitalWrite(A2, LOW);
  }
}


void doCommands() {
   int incoming = Serial.read();
    if(incoming != 13) {
      command = command += (char)incoming;
    } else { // command finished
      String outCommand = "";
      if(command == "hi") {
        hiLdr = ldr;
        outCommand = outCommand + "hi: ";
        outCommand = outCommand + hiLdr;
        setThreshold();
        Serial.println(outCommand);
      } else if (command == "lo") {
        loLdr = ldr;
        outCommand = outCommand + "lo: ";
        outCommand = outCommand + loLdr;
        setThreshold();
        Serial.println(outCommand);
      } else if (command == "stat") {
        outCommand = outCommand + "lo: " + loLdr + " hi: "+hiLdr+" threshold: "+threshold;
        Serial.println(outCommand);
      } else if (command == "meas") {
        Serial.println("Measuring...");
        state = STATE_START_MEASURING;
        shutterOpen = 0;
        shutterClose = 0;
        highest = 0;
      } else if (command == "canc") {
        Serial.println("Cancelling");
        state = STATE_READY;
      } else if (command == "cal") {
        Serial.println("Calibrating");
        state = STATE_CALIBRATING;
        hiLdr = 0;
        loLdr = 1024;
      } else if (command == "las") {
        laser(true);
      } else if (command == "lasoff") {
        laser(false);
      }
      else {
        outCommand += "Unknown command: [";
        outCommand += command;
        outCommand += "]";
        Serial.println(outCommand);
      }
      command = "";      
    }
}


