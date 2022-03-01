#include <CAN.h>
#include <ESP_FlexyStepper.h>
#include "Controller_Contants.h"

const bool SERIAL_ON = true;
const int proccesDelay = 0;

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
void IRAM_ATTR Increment_Translation();
void IRAM_ATTR Increment_Rotation();

int ID = 0;

double translationMeasured = 0;
double rotationMeasured = 0;
double translationDesired = 0;
double rotationDesired = 0;

int translationEncoder = 0;
int rotationEncoder = 0;
bool rotationEncoderActive = false;
bool translationEncoderActive = false;

unsigned long messageTime = 0;
bool RUNNING = false;
bool E_STOP = false;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} translationMeasured_f;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} rotationMeasured_f;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} translationDesired_f;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} rotationDesired_f;

void setup() {
    if (SERIAL_ON) {
        Serial.begin(115200);
        while (!Serial);
    }
    
    //Pin Initialization
    pinMode(ID_2, INPUT);
    pinMode(ID_1, INPUT);
    attachInterrupt(TRANSLATION_ENCODER, Increment_Translation, RISING);
    attachInterrupt(ROTATION_ENCODER, Increment_Rotation, RISING);
    CAN.setPins(RXR_CAN, TXD_CAN);
    pinMode(ALL_GOOD_LED, OUTPUT);
    Translation_Stepper.connectToPins(TRANSLATION_DRIVER_PULSE, TRANSLATION_DRIVER_DIR);
    Rotation_Stepper.connectToPins(ROTATION_DRIVER_PULSE, ROTATION_DRIVER_DIR);

    ID = digitalRead(ID_2)*2 + digitalRead(ID_1);

    if (SERIAL_ON) {
        Serial.print("Board ID: ");
        Serial.println(ID);
    }

    Translation_Stepper.setStepsPerMillimeter(STEP_PULSE_TRANSLATION_CONVERSION);
    Translation_Stepper.setAccelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);
    Translation_Stepper.setDecelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);
    Translation_Stepper.setSpeedInMillimetersPerSecond(maxSpeedTranslation);

    Rotation_Stepper.setStepsPerMillimeter(STEP_PULSE_ROTATION_CONVERSION);
    Rotation_Stepper.setAccelerationInMillimetersPerSecondPerSecond(maxAccelerationRotation);
    Rotation_Stepper.setDecelerationInMillimetersPerSecondPerSecond(maxAccelerationRotation);
    Rotation_Stepper.setSpeedInMillimetersPerSecond(maxSpeedRotation);

    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        if (SERIAL_ON) Serial.println("Starting CAN failed!");
        digitalWrite(ALL_GOOD_LED, LOW);
        while (1);
    }
    if (SERIAL_ON) Serial.println("Starting CAN success");
    CAN.filter(0b10101111, 1 << ID);
    CAN.onReceive(CAN_Handler);
    if (!ZERO()){
        digitalWrite(ALL_GOOD_LED, LOW);
        while(1);
    }
    
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

bool ZERO(){
    if (SERIAL_ON) Serial.println("Zeroing...");
    bool success = (Translation_Stepper.moveToHomeInMillimeters(homeDirTranslation, homeSpeedTranslation, maxTranslations[ID], TRANSLATION_DRIVER_ZERO) 
    &&
    Rotation_Stepper.moveToHomeInRevolutions(homeDirRotation, homeSpeedRotation, 1, ROTATION_DRIVER_ZERO));
    if (success) translationEncoder = rotationEncoder = 0;
    if (SERIAL_ON && success) Serial.println("Zeroing success");
    else if(SERIAL_ON) Serial.println("Zeroing failed");
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
        if (CAN_running()) {
            if (translationDesired > maxTranslations[ID]) translationDesired = maxTranslations[ID];
            if (translationDesired < 0) translationDesired = 0;
            Translation_Stepper.setTargetPositionInMillimeters(translationDesired);
            Rotation_Stepper.setTargetPositionInRevolutions(rotationDesired);
        }

        if (rotationEncoderActive) Rotation_Stepper.setCurrentPositionInRevolutions(rotationEncoder);
        if (translationEncoderActive) Translation_Stepper.setCurrentPositionInMillimeters(translationEncoder);
        delay(proccesDelay);
    }
}

//Need to decide on packet format
void CAN_Sender(void * parameters){
    for(;;){
        translationMeasured_f.value = (float)Translation_Stepper.getCurrentPositionInMillimeters();
        rotationMeasured_f.value = (float)Rotation_Stepper.getCurrentPositionInRevolutions();;
        if (CAN_running()) {
            CAN.beginPacket(1 << ID);
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(translationMeasured_f.bytes[i]);
            }
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(rotationMeasured_f.bytes[i]);
            }
            CAN.endPacket();
        }
        if (!RUNNING) {
            CAN.beginPacket(1 << ID + 0b010000);
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(translationMeasured_f.bytes[i]);
            }
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(rotationMeasured_f.bytes[i]);
            }
            CAN.endPacket();
        }
        delay(COM_DELAY);
    }
}

//Need to decide on packet format
void CAN_Handler(int packet_size){
    messageTime = millis();
    if (SERIAL_ON) {
        Serial.print("(packet: 0x");
        Serial.print(CAN.packetId(), HEX);
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
        if (translationIndex > 0){
            translationDesired_f.bytes[translationIndex--] = (byte)CAN.read();
        } else if (rotationIndex > 0) {
            rotationDesired_f.bytes[rotationIndex--] = (byte)CAN.read();
        } else {
            if (SERIAL_ON) Serial.println("DATA DROPPED)");
            return;
        }
    }
    if (dataReceived){
        if (!((translationIndex > 0) && (rotationIndex > 0))) {
            if (SERIAL_ON) Serial.println("LESS THAN 8 BYTES RECEIVED)");
            return;
        }
        rotationDesired = (double)rotationDesired_f.value;
        translationDesired = (double)translationDesired_f.value;
        if (SERIAL_ON){
            Serial.print(" Rotation: ");
            Serial.print(rotationDesired);
            Serial.print(" Translation: ");
            Serial.print(translationDesired);
        }
    }

    if (SERIAL_ON) Serial.println(")");
    if (!RUNNING) START();
}

void IRAM_ATTR Increment_Translation(){
    int spinDirection = Translation_Stepper.getDirectionOfMotion();
    if (spinDirection > 0) {
        translationEncoder += 1 * ENCODER_PULSE_TRANSLATION_CONVERSION;
    } else if (spinDirection < 0) {
        translationEncoder -= 1 * ENCODER_PULSE_TRANSLATION_CONVERSION;
    } else {
        if (SERIAL_ON) Serial.println("Increment_Translation called with movement = 0");
    }
    if (!translationEncoderActive && abs(translationEncoder) > MINIMUM_ENCODER_PULSES * ENCODER_PULSE_TRANSLATION_CONVERSION) {
        if (SERIAL_ON) Serial.println("Translation Encoder Active");
        translationEncoderActive = true;
    }
}

void IRAM_ATTR Increment_Rotation(){
    int spinDirection = Rotation_Stepper.getDirectionOfMotion();
    if (spinDirection > 0) {
        rotationEncoder += 1 * ENCODER_PULSE_ROTATION_CONVERSION;
    } else if (spinDirection < 0) {
        rotationEncoder -= 1 * ENCODER_PULSE_ROTATION_CONVERSION;
    } else {
        if (SERIAL_ON) Serial.println("Increment_Rotation called with movement = 0");
    }
    if (!rotationEncoderActive && abs(rotationEncoder) > MINIMUM_ENCODER_PULSES * ENCODER_PULSE_ROTATION_CONVERSION) {
        if (SERIAL_ON) Serial.println("Rotation Encoder Active");
        rotationEncoderActive = true;
    }
}
