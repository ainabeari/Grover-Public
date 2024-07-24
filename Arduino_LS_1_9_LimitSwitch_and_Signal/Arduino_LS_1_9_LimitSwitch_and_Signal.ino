//This has an input trigger, when it is high it will make the motor turn the other way//

#include <TMCStepper.h>
//NEEDS COMMON GROUND WITH S3
#define LIMIT_SWITCH_TOP  2 // Limit switch has to be digital
#define LIMIT_SWITCH_BOTTOM  3 // Limit switch has to be digital
#define INPUT_TRIGGER_PIN 4 // Choose a digital pin for the input trigger (e.g., pin 3)s D4 PIN on arduino nano



#define MAX_SPEED         4//40 // In timer value
#define MIN_SPEED         1000

#define STALL_VALUE       0 // [-64..63]

#define EN_PIN            7  // Enable
#define DIR_PIN           5  // Direction
#define STEP_PIN          6  // Step
#define CS_PIN            10 // Chip select
#define R_SENSE           0.11f // Match to your driver
bool clockwise = true; // Initially move clockwise
//bool initialMove = false;

TMC2130Stepper driver(CS_PIN, R_SENSE); // Hardware SPI

#define STEP_PORT     PORTF // Match with STEP_PIN
#define STEP_BIT_POS  0      // Match with STEP_PIN

ISR(TIMER1_COMPA_vect) {
  digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
}

void setup() {
  SPI.begin();
  Serial.begin(250000);
  while (!Serial);

  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(MISO, INPUT_PULLUP);
  digitalWrite(EN_PIN, HIGH); // Initially, set motor to complete stop

  driver.begin();
  driver.toff(4);
  driver.blank_time(20);
  driver.rms_current(700); // mA
  driver.microsteps(16);
  driver.en_pwm_mode(true);
  driver.pwm_autoscale(true);
  driver.TCOOLTHRS(0xFFFFF); // 20bit max
  driver.THIGH(0);
  driver.semin(5);
  driver.semax(2);
  driver.sedn(0b01);
  driver.sgt(STALL_VALUE);

  cli(); // stop interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 256;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11);
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts

  pinMode(LIMIT_SWITCH_TOP, INPUT_PULLUP);
  //pinMode(LIMIT_SWITCH_TOP, INPUT);

  pinMode(LIMIT_SWITCH_BOTTOM, INPUT_PULLUP);
  pinMode(INPUT_TRIGGER_PIN, INPUT);

  //INPUT_PULLUP needs to be added for the limit switches to work properly
 
}

/*ALL OF THE PINS FOR LIMIT SWITCH + SIGNAL START HIGH*/
/*INITIALLY SET ALL VALUES TO 1 (HIGH)*/

/*INPUT TRIGGER PIN HAS TO START HIGH, SEND INITIAL 1 AT 3V3 
WHEN IT IS 0 THIS MEANS A SIGNAL HAS BEEN SENT. NEEDS A 0 FROM THE OTHER ARDUINO TO START MOVING
ONLY NEEDS ONE 0*/

/*TOP LIMIT SWITCH: NEEDS TO BE ON HIGH UNTIL THE LIMIT SWITCH IS HIT. ONLY ONE 0 IS REQUIRED TO MAKE IT STAY IN THAT STATE
ONLY NEEDS ONE 0
*/

/*BOTTOM LIMIT SWITCH: NEEDS TO BE ON HIGH UNTIL THE LIMIT SWITCH IS HIT. ONLY ONE 0 IS REQUIRED TO MAKE IT STAY IN THAT STATE
ONLY NEEDS ONE 0
*/

void loop() {
  static uint32_t last_switch_time = 0;
  static bool initialMove = true;

  // If moving clockwise and top limit switch is hit, stop the motor
  if (clockwise && digitalRead(LIMIT_SWITCH_TOP) == LOW){
    
    //!!!TOP LIMIT SWITCH HAS BEEN HIT!!!//
    //!!!ENABLE LED IS ON, DIRECTION IS OFF!!!//

    //Make it go counterclockwise for a lil
    digitalWrite(DIR_PIN, HIGH); // Set direction to counterclockwise
    digitalWrite(EN_PIN, LOW); // Start the motor
    Serial.println("clockwise but ...,Counterclockwise for a lil");
    delay(1000); // Run counterclockwise for 1 second


    digitalWrite(EN_PIN, HIGH); // Stops the motor
    //clockwise = false; // Set-up for counterclockwise
    Serial.println("Enable should be ..., hit the top and stop");
    //Serial.println(digitalRead(INPUT_TRIGGER_PIN));
    //delay(1000);
    clockwise = false;
  }
  
  if (digitalRead(INPUT_TRIGGER_PIN) == LOW && clockwise) {
    
    ///!!!A SIGNAL HAS BEEN SENT!!!///
    ///!!!BOTH LEDs ARE OFF !!!///

    digitalWrite(DIR_PIN, LOW); // Set direction to clockwise
    digitalWrite(EN_PIN, LOW); // Start the motor

    clockwise = true; // Set direction to clockwise
    Serial.println("Enable should be ... going clockwise, no lim switch");
    delay(1000);
  }
  

    // If moving counterclockwise and bottom limit switch is hit, stop the motor
  if (!clockwise && digitalRead(LIMIT_SWITCH_BOTTOM) == LOW) {

      ///!!!BOTTOM LIMIT SWITCH HAS BEEN HIT!!!///
      ///!!!BOTH LEDs ARE ON!!!///

      //Make it go clockwise for a lil
      digitalWrite(DIR_PIN, LOW); // Set direction to clockwise
      digitalWrite(EN_PIN, LOW); // Start the motor
      Serial.println("Counterclockwise but ...,clockwise for a lil");
      delay(1000); // Run counterclockwise for 1 second

      digitalWrite(EN_PIN, HIGH); // Stop the motor
      clockwise = true; // Set direction to clockwise
      Serial.println("Enable should be ..., hit the bottom");
      delay(1000);
  }

  if (digitalRead(INPUT_TRIGGER_PIN) == LOW && !clockwise) {

    ///!!!A SIGNAL HAS BEEN SENT!!!///
    ///!!!ENABLE LED IS OFF, DIRECTION IS ON!!!///

    digitalWrite(DIR_PIN, HIGH); // Set direction to counterclockwise
    digitalWrite(EN_PIN, LOW); // Start the motor
    
    clockwise = false; // Set direction to counterclockwise
    Serial.println("Enable should be ..., going counterclockwise, no lim switch");
    delay(1000);
  }

  if (clockwise && initialMove && digitalRead(INPUT_TRIGGER_PIN) == HIGH) {

    //!!!BOTH LEDs ARE OFF!!!///

    digitalWrite(DIR_PIN, LOW); // Set direction to clockwise
    digitalWrite(EN_PIN, LOW); // Start the motor
    clockwise = true; // Set direction to clockwise
    initialMove = false; 
    Serial.println("Performing Initial Move, going clockwise");
    delay(1000);
    
  }
}
