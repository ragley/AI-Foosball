#include <ESP_FlexyStepper.h>

#define PULSE_1 23
#define DIRECTION_1 22
#define ANALOG 4
#define CORE 1

const float stepPerRevolution = 1;

ESP_FlexyStepper stepper_1;

void setup() {
  stepper_1.connectToPine(PULSE_1, DIRECTION_1);
  stepper_1.setStepsPerRevolution(stepPerRevolution);
  stepper_1.setDirectionToHome();

  stepper_1.startAsService(CORE);
}

void loop() {
}
