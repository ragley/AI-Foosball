#include <ESP_FlexyStepper.h>
#include "Controller_Constants.h"

#define CORE 0

const int REVOLUTIONS = 1;

int target_ROT = REVOLUTIONS;
int target_TRANS = REVOLUTIONS*mmPerRevolution;
int direction_ROT = 1;
int direction_TRANS = 1;
double timer_ROT = millis();
double timer_TRANS = millis();



ESP_FlexyStepper Rotational_Driver;
ESP_FlexyStepper Translation_Driver;

void setup() {
  Serial.begin(115200);
  Rotational_Driver.connectToPins(ROTATION_DRIVER_PULSE, ROTATION_DRIVER_DIR);
  Rotational_Driver.setStepsPerRevolution(STEP_PULSE_ROTATION_CONVERSION);
  Rotational_Driver.setSpeedInRevolutionsPerSecond(maxSpeedRotation);
  Rotational_Driver.setAccelerationInRevolutionsPerSecondPerSecond(maxAccelerationRotation);
  Rotational_Driver.setDecelerationInRevolutionsPerSecondPerSecond(maxAccelerationRotation);
  Rotational_Driver.setTargetPositionToStop();
  Rotational_Driver.startAsService(CORE);

  Translation_Driver.connectToPins(TRANSLATION_DRIVER_PULSE, TRANSLATION_DRIVER_DIR);
  Translation_Driver.setStepsPerMillimeter(STEP_PULSE_TRANSLATION_CONVERSION);
  Translation_Driver.setSpeedInMillimetersPerSecond(maxSpeedTranslation);
  Translation_Driver.setAccelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);
  Translation_Driver.setDecelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);
  Translation_Driver.setTargetPositionToStop();
  Translation_Driver.startAsService(CORE);
}

void loop() {
  Serial.print("ROT __ ACC: ");
  Serial.print(maxAccelerationRotation);
  Serial.print(" SPEED: ");
  Serial.print(Rotational_Driver.getCurrentVelocityInRevolutionsPerSecond());
  Serial.print(" REV: ");
  Serial.print(Rotational_Driver.getCurrentPositionInRevolutions());
  Serial.print(" TARGET: ");
  Serial.print(Rotational_Driver.getTargetPositionInRevolutions());
  Serial.print(" TRANS __ ACC: ");
  Serial.print(maxAccelerationTranslation);
  Serial.print(" SPEED: ");
  Serial.print(Translation_Driver.getCurrentVelocityInRevolutionsPerSecond());
  Serial.print(" REV: ");
  Serial.print(Translation_Driver.getCurrentPositionInRevolutions());
  Serial.print(" TARGET: ");
  Serial.println(Translation_Driver.getTargetPositionInRevolutions());

  if (Rotational_Driver.getDistanceToTargetSigned() == 0 && direction_ROT*target_ROT > 0) {
    direction_ROT *= -1;
    timer_ROT = millis() + 1000;
  }
  if (millis() > timer_ROT && direction_ROT*target_ROT < 0) {
    target_ROT =  direction_ROT*REVOLUTIONS;
    Rotational_Driver.setTargetPositionInRevolutions(target_ROT);
    timer_ROT = millis() + 1000;
  }

  if (Translation_Driver.getDistanceToTargetSigned() == 0 && direction_TRANS*target_TRANS > 0) {
    direction_TRANS *= -1;
    timer_TRANS = millis() + 1000;
  }
  if (millis() > timer_TRANS && direction_TRANS*target_TRANS < 0) {
    target_TRANS = direction_TRANS*REVOLUTIONS*mmPerRevolution;
    Translation_Driver.setTargetPositionInMillimeters(target_TRANS);
    timer_TRANS = millis() + 1000;
  }
}
