// Ported to ATtiny1616 (megaTinyCore) from the Uno R3 + MCP4725 version
// originally uploaded to module 8-4-18.
//
// Pin map:
//   PA5 - charhead (sensitivity knob)   was A3
//   PA4 - timehead (timebase knob)      was A2
//   PB4 - gate output                   was D2
//   PA6 - DAC0 output (fixed in silicon; not reassignable)
//   PA3 - reset/break input             was D4  <-- not specified, change if needed
//   PB2/PB3 - Serial TX/RX (megaTinyCore default USART0 pins on the 20-pin part)
//   PA0 - UPDI (programming), leave free

#define CHARHEAD_SENSITIVITY  PIN_PA5 //analog read pot
#define TIMEHEAD   PIN_PA4  //analog read pot
#define GATE_PIN   PIN_PB1
#define RESET_PIN  PIN_PA3

long divNumber;
int x = 0;
int y = 0;
int j = 0;
int stopMe = 0;

// ---------------------------------------------------------------------------
// DAC0 setup. On tinyAVR 1-series the DAC reference comes from VREF and can
// only be 0.55, 1.1, 1.5, 2.5 or 4.34 V -- there is no VDD option for the DAC.
// 4.34 V is used here, so full scale (255) is about 4.32 V, not 5 V.
// ---------------------------------------------------------------------------
void dacInit() {
  // Select 4.34 V reference for DAC0 without touching the ADC reference bits
  VREF.CTRLA = (VREF.CTRLA & ~VREF_DAC0REFSEL_gm) | VREF_DAC0REFSEL_4V34_gc;
  VREF.CTRLB |= VREF_DAC0REFEN_bm;              // keep the DAC reference running

  // Datasheet recommendation: disable the digital input buffer on the DAC pin
  PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;

  DAC0.DATA  = 0;
  DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;    // enable DAC and drive PA6
}

inline void dacWrite8(uint8_t value) {
  DAC0.DATA = value;                            // 0..255
}

void setup() {
  pinMode(GATE_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT);                    // same as original: external pull-down assumed
  pinMode(CHARHEAD_SENSITIVITY, INPUT);
  pinMode(TIMEHEAD, INPUT);

  Serial.begin(9600);

  dacInit();
}

void loop() {

  long charhead = analogRead(CHARHEAD_SENSITIVITY);         // 10-bit, VDD reference (megaTinyCore default)
  long timehead = analogRead(TIMEHEAD);

  divNumber = random(1023);

  if (charhead > divNumber) {
    x++;
  //  Serial.println("you are in counter augment x is");
   // Serial.println(x);
  } else {
    x = 0;
  }

  if ((x == 0) && (y == 0)) {
    digitalWrite(GATE_PIN, HIGH);
    dacWrite8(divNumber >> 2);                  // 0..1022 -> 0..255 (was divNumber*4 for 12-bit)
    y = y + 1;
  }

  if (x == 1) {
    digitalWrite(GATE_PIN, LOW);
    dacWrite8(divNumber >> 2);
    y = 0;
  }
 
  for (j = 0; j < (timehead * 10); j++) {
    delay(1);
    stopMe = digitalRead(RESET_PIN);
    if (stopMe == HIGH) {
   //   Serial.println("RESET COUNTER ACTIVATED");
      break;
    }
  }
}
