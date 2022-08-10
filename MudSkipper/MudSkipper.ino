#define WATER_SENSOR 11
#define INLET_MOTOR 10
#define ROT_A 9
#define ROT_B 8
#define ROT_SW 7
#define SEG_CLK 4
#define SEG_DIO 6

#define SERVO 3

#include <TM1637Display.h>
#include <Servo.h>

TM1637Display display(SEG_CLK, SEG_DIO);
Servo valve;

const uint8_t all_on[] = {0xff, 0xff, 0xff, 0xff};
const uint8_t done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

int counter = 0;

//EDITABLE VALUES 
int disp_count = 200;
int closed = 90;
int open = 0;
//-------------

int presentState;
int previousState;
bool count_active = false;
int sleep_counter = 0;
int user_input = 0;
int countdown_disp = 0;
bool active = false;
void setup() {
  Serial.begin(9600);
  pinMode(WATER_SENSOR, INPUT);
  pinMode(INLET_MOTOR, OUTPUT);
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  pinMode(ROT_SW, INPUT);
  display.setBrightness(3);
  display.clear();
  valve.attach(3);
  previousState = digitalRead(ROT_A);
  Serial.println("Testing 33");
}


void loop() {
  Serial.println("testing 2");
  auto sw_push = digitalRead(ROT_SW);
  valve.write(90);
  digitalWrite(INLET_MOTOR, LOW);
  readEncoder();
  user_input = disp_count + 10 * counter;
  if (!count_active) {
    display.showNumberDec(user_input);
    Serial.println(user_input);
  }
  if (!sw_push) {
    display.setSegments(done);
    Serial.println("done");
    active = true;
    delay(1000);
    if (count_active) {
      count_active = false;
    } else {
      sleep_counter = 0;
      count_active = true;
    }

  }
  int state = 0; //0 is counting, 1 is filling, 2 is draining
  bool tank_filling = false;
  while (active) {
    auto sw_push = digitalRead(ROT_SW);
    auto water_level = digitalRead(WATER_SENSOR);
    valve.write(open);
    if (!sw_push) {
      active = false;
      Serial.println("Stopping...");
      delay(1000);
      sleep_counter = 0;
    }
    countdown_disp = user_input - sleep_counter;
    display.showNumberDec(countdown_disp);
    Serial.println(countdown_disp);
    unsigned int timer = 1000 * 60;
    if (state == 0) {
      delay(timer);
      sleep_counter++;
    }
    Serial.println(sleep_counter);
    if (sleep_counter >= user_input && tank_filling == false) {
      state = 1;
      Serial.println("Filling Tank");
      digitalWrite(INLET_MOTOR, HIGH);
      if (water_level) {
        state = 0;
        digitalWrite(INLET_MOTOR, LOW);
        Serial.println("TANK FILLED");
        tank_filling = true;
      }
    }
    if (sleep_counter >= 2 * user_input && state == 0) {
      valve.write(closed); // Valve open
      state = 2;
      Serial.println("TANK DRAINING");
      sleep_counter = 0;
      unsigned int timer2 = 3600 * 1000;
      delay(timer2); //let the tank drain
      state = 0;
      tank_filling = false;
    }


  }

}

void readEncoder() {
  presentState = digitalRead(ROT_A);
  if (presentState != previousState)
  {
    if (digitalRead(ROT_B) != presentState)
    {
      counter ++;
    }
    else {
      counter --;
    }
  }
  previousState = presentState; // Replace previous state of the ROT_A with the current state
}
