#include <CAN.h>
#include <ESP_FlexyStepper.h>
#include "Controller_Constants.h"

const bool SERIAL_ON = true;
const int proccesDelay = COM_DELAY;

TaskHandle_t Controller;
TaskHandle_t Communication;

ESP_FlexyStepper Translation_Stepper;
ESP_FlexyStepper Rotation_Stepper;

void SHUTDOWN();
void START();
bool CAN_running();
void Controller_Core(void * parameters);
void CAN_Sender(void * parameters);
void CAN_Handler(int packet_size);
// void IRAM_ATTR Increment_Translation();
// void IRAM_ATTR Increment_Rotation();

int boardID = 0;

double translationMeasured = 0;
double rotationMeasured = 0;
double translationDesired = 0;
double rotationDesired = 0;

int translationSensor = 0;
int rotationSensor = 0;
bool rotationSensorActive = false;
bool translationSensorActive = false;

unsigned long messageTime = 0;
bool RUNNING = false;
bool E_STOP = false;
bool ENABLED = false;

typedef union {
    float value;
    byte bytes[sizeof(float)];
} FLOAT_BYTE_UNION;

void setup() {
    if (SERIAL_ON) {
        Serial.begin(115200);
        while (!Serial);
    }
    
    //Pin Initialization
    pinMode(ID_2, INPUT);
    pinMode(ID_1, INPUT);
    // attachInterrupt(TRANSLATION_SENSOR, Increment_Translation, RISING);
    // attachInterrupt(ROTATION_SENSOR, Increment_Rotation, RISING);
    CAN.setPins(RXR_CAN, TXD_CAN);
    pinMode(ALL_GOOD_LED, OUTPUT);
    Translation_Stepper.connectToPins(TRANSLATION_DRIVER_PULSE, TRANSLATION_DRIVER_DIR);
    Rotation_Stepper.connectToPins(ROTATION_DRIVER_PULSE, ROTATION_DRIVER_DIR);

    boardID = digitalRead(ID_2)*2 + digitalRead(ID_1);

    if (SERIAL_ON) {
        Serial.print("Board ID: ");
        Serial.println(boardID);
    }

    Translation_Stepper.setStepsPerMillimeter(STEP_PULSE_TRANSLATION_CONVERSION);
    Translation_Stepper.setAccelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);
    Translation_Stepper.setDecelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);

    Rotation_Stepper.setStepsPerRevolution(STEP_PULSE_ROTATION_CONVERSION);
    Rotation_Stepper.setAccelerationInRevolutionsPerSecondPerSecond(maxAccelerationRotation);
    Rotation_Stepper.setDecelerationInRevolutionsPerSecondPerSecond(maxAccelerationRotation);

    // if (!ZERO()){
    //     digitalWrite(ALL_GOOD_LED, LOW);
    //     while(1);
    // }

    Translation_Stepper.setSpeedInMillimetersPerSecond(maxSpeedTranslation);
    Rotation_Stepper.setSpeedInRevolutionsPerSecond(maxSpeedRotation);

    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        if (SERIAL_ON) Serial.println("Starting CAN failed!");
        digitalWrite(ALL_GOOD_LED, LOW);
        while (1);
    }
    if (SERIAL_ON) Serial.println("Starting CAN success");
    // CAN.filter(0b00001111, 0b10100000 + (1 << boardID));
    CAN.filter(0b00001111, 0b10100001);
    CAN.onReceive(CAN_Handler);
    
    messageTime = millis();
    //Assigning Controller and Communication Tasks to specific cores
    xTaskCreatePinnedToCore(Controller_Core,
                            "Controller",
                            10000,
                            NULL,
                            5,
                            NULL,
                            CONTROL_CORE);

    xTaskCreatePinnedToCore(CAN_Sender,
                            "Communication",
                            10000,
                            NULL,
                            5,
                            NULL,
                            COM_CORE);
    START();
}

void loop() {
}

bool enabled(){
  if (digitalRead(ENABLE) == LOW){
    ENABLED = false;
    return ENABLED;
  } else {
    ENABLED = true;
    return ENABLED;
  }
}

bool ZERO(){
    if (SERIAL_ON) Serial.print("Zeroing Translation...");
    bool successTrans = Translation_Stepper.moveToHomeInMillimeters(directions[boardID][TRANSLATION]*-1, homeSpeedTranslation, maxTranslations[boardID], TRANSLATION_DRIVER_ZERO);
    if (!successTrans) {
        if (SERIAL_ON) Serial.println(" Failed");
        return successTrans;
    }
    if (SERIAL_ON) {
        Serial.println(" Success");
        Serial.print("Zeroing Rotation...");
    }
    bool successRot = Rotation_Stepper.moveToHomeInRevolutions(directions[boardID][ROTATION]*-1, homeSpeedRotation, 2, ROTATION_DRIVER_ZERO);
    if (!successRot) {
        if (SERIAL_ON) Serial.println(" Failed");
        return successRot;
    }
    bool success = successRot && successTrans;
    if (success) {
        translationSensor = rotationSensor = 0;
    }
    if (SERIAL_ON && success) Serial.println(" Success");
    else if(SERIAL_ON) Serial.println(" Failed");
    return success;
}

void SHUTDOWN() {
    if (RUNNING) {
        Translation_Stepper.emergencyStop(false);
        Rotation_Stepper.emergencyStop(false);
        Translation_Stepper.stopService();
        Rotation_Stepper.stopService();

        digitalWrite(ALL_GOOD_LED, LOW);
        RUNNING = false;

        if (SERIAL_ON) Serial.println("SHUTDOWN");
    }
}

void START() {
    if (!RUNNING) {
        //Activating Stepper
        Translation_Stepper.startAsService(STEPPER_CORE);
        Rotation_Stepper.startAsService(STEPPER_CORE);

        digitalWrite(ALL_GOOD_LED, HIGH);
        RUNNING = true;
        E_STOP = false;
        if (SERIAL_ON) Serial.println("START");
    }
}

bool CAN_running(){
    if (millis() - messageTime > MAX_COM_DELAY) {
        SHUTDOWN();
        return false;
    } else {
        if (!E_STOP) START();
        return true;
    }
}

void Controller_Core(void * parameters){
    for(;;){
        double startTime = millis();
        if (CAN_running() && enabled()) {
            if (translationDesired > maxTranslations[boardID]) translationDesired = maxTranslations[boardID];
            if (translationDesired < 0) translationDesired = 0;
            Translation_Stepper.setTargetPositionInMillimeters(directions[boardID][TRANSLATION]*translationDesired);
            Rotation_Stepper.setTargetPositionInRevolutions(directions[boardID][ROTATION]*rotationDesired);
        }

        // if (rotationSensorActive) Rotation_Stepper.setCurrentPositionInRevolutions(rotationSensor);
        // if (translationSensorActive) Translation_Stepper.setCurrentPositionInMillimeters(translationSensor);
        double controllerExecutionTime = millis() - startTime;
        delay(proccesDelay - controllerExecutionTime);
    }
}

//Need to decide on packet format
void CAN_Sender(void * parameters){
    FLOAT_BYTE_UNION translationMeasured_f;
    FLOAT_BYTE_UNION rotationMeasured_f;

    for(;;){
        double startTime = millis();
        translationMeasured_f.value = (float)Translation_Stepper.getCurrentPositionInMillimeters();
        rotationMeasured_f.value = (float)Rotation_Stepper.getCurrentPositionInRevolutions();;
        if (CAN_running()) {
            CAN.beginPacket(0b10010000 + (1 << boardID));
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(translationMeasured_f.bytes[i]);
            }
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(rotationMeasured_f.bytes[i]);
            }
            CAN.endPacket();
        }
        if (!RUNNING || !ENABLED) {
            CAN.beginPacket((1 << boardID) + 0b10000000);
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(translationMeasured_f.bytes[i]);
            }
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(rotationMeasured_f.bytes[i]);
            }
            CAN.endPacket();
        }
        double comExecutionTime = millis() - startTime;
        delay(COM_DELAY - comExecutionTime);
    }
}

//Need to decide on packet format
void CAN_Handler(int packet_size){
    FLOAT_BYTE_UNION translationDesired_f;
    FLOAT_BYTE_UNION rotationDesired_f;
    messageTime = millis();
    if (SERIAL_ON) {
        Serial.print("(packet: 0x");
        Serial.print(CAN.packetId(), HEX);
        Serial.print(" ");
    }

    //EMERGENCY STOP
    if (CAN.packetId() & 0b11110000 == 0b00000000) {
        SHUTDOWN();
        E_STOP = true;
        return;
    }

    //ZERO
    if (CAN.packetId() & 0b11000000 == 0b01000000) {
        SHUTDOWN();
        ZERO();
        START();
        return;
    }

    int rotationIndex = sizeof(float)-1;
    int translationIndex = sizeof(float)-1;
    bool dataReceived = false;
    while(CAN.available()){
        dataReceived = true;
        if (translationIndex >= 0){
            translationDesired_f.bytes[translationIndex--] = (byte)CAN.read();
        } else if (rotationIndex >= 0) {
            rotationDesired_f.bytes[rotationIndex--] = (byte)CAN.read();
        } else {
            if (SERIAL_ON) Serial.println("DATA DROPPED)");
            return;
        }
    }
    if (dataReceived){
        if (!((translationIndex < 0) && (rotationIndex < 0))) {
            if (SERIAL_ON) Serial.println("LESS THAN 8 BYTES RECEIVED)");
            return;
        }
        rotationDesired = (double)rotationDesired_f.value;
        translationDesired = (double)translationDesired_f.value;
        if (SERIAL_ON){
            Serial.print(" Rotation: ");
            //Serial.print(rotationDesired);
            Serial.print(rotationDesired_f.value);
//            Serial.print(rotationDesired_f.bytes[0]);
//            Serial.print(rotationDesired_f.bytes[1]);
//            Serial.print(rotationDesired_f.bytes[2]);
//            Serial.print(rotationDesired_f.bytes[3]);
            Serial.print(" Translation: ");
            //Serial.print(translationDesired);
            Serial.print(translationDesired_f.value);
//            Serial.print(translationDesired_f.bytes[0]);
//            Serial.print(translationDesired_f.bytes[1]);
//            Serial.print(translationDesired_f.bytes[2]);
//            Serial.print(translationDesired_f.bytes[3]);

        }
    }

    if (SERIAL_ON) Serial.println(")");
    if (!RUNNING) START();
}

// void IRAM_ATTR Increment_Translation(){
//     int spinDirection = Translation_Stepper.getDirectionOfMotion();
//     if (spinDirection > 0) {
//         translationSensor += 1 * SENSOR_PULSE_TRANSLATION_CONVERSION;
//     } else if (spinDirection < 0) {
//         translationSensor -= 1 * SENSOR_PULSE_TRANSLATION_CONVERSION;
//     } else {
//         if (SERIAL_ON) Serial.println("Increment_Translation called with movement = 0");
//     }
//     if (!translationSensorActive && abs(translationSensor) > MINIMUM_SENSOR_PULSES * SENSOR_PULSE_TRANSLATION_CONVERSION) {
//         if (SERIAL_ON) Serial.println("Translation Sensor Active");
//         translationSensorActive = true;
//     }
// }

// void IRAM_ATTR Increment_Rotation(){
//     int spinDirection = Rotation_Stepper.getDirectionOfMotion();
//     if (spinDirection > 0) {
//         rotationSensor += 1 * SENSOR_PULSE_ROTATION_CONVERSION;
//     } else if (spinDirection < 0) {
//         rotationSensor -= 1 * SENSOR_PULSE_ROTATION_CONVERSION;
//     } else {
//         if (SERIAL_ON) Serial.println("Increment_Rotation called with movement = 0");
//     }
//     if (!rotationSensorActive && abs(rotationSensor) > MINIMUM_SENSOR_PULSES * SENSOR_PULSE_ROTATION_CONVERSION) {
//         if (SERIAL_ON) Serial.println("Rotation Sensor Active");
//         rotationSensorActive = true;
//     }
// }
