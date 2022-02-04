#include <ESP_FlexyStepper.h>

#define PULSE_1 23
#define DIRECTION_1 22
#define ANALOG 4
#define CORE 1

const float stepPerRevolution = 100; //according to the 1.80 degree step angle from data sheet
const int maxRPM = 2000; //set by library as max speed
const int maxAnalog = 2048*2;
const int maxACC = 100;
const int GOAL = 10000;

int RPM;

ESP_FlexyStepper stepper_1;

void setup() {
  Serial.begin(115200);
  stepper_1.connectToPins(PULSE_1, DIRECTION_1);
  stepper_1.setStepsPerRevolution(stepPerRevolution);
  stepper_1.setAccelerationInRevolutionsPerSecondPerSecond(maxACC);
  stepper_1.setDecelerationInRevolutionsPerSecondPerSecond(maxACC);

  stepper_1.startAsService(CORE);
}

void loop() {
  RPM = analogRead(ANALOG)*maxRPM/maxAnalog;
  stepper_1.setSpeedInRevolutionsPerSecond(RPM);
  Serial.print("RPM: ");
  Serial.print(RPM);
  Serial.print(" SPEED: ");
  Serial.println(stepper_1.getCurrentVelocityInRevolutionsPerSecond());
  if (RPM == 0) {
    stepper_1.setTargetPositionToStop();
  } else {
    stepper_1.setTargetPositionRelativeInRevolutions(GOAL);
  }
  
}
