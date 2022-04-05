#include <CAN.h>
#include <ESP_FlexyStepper.h>
#include "Controller_Constants.h"

//States
#define OFF 0
#define RUNNING 1
#define DISABLED 2
#define EMERGENCY_STOP 3
#define DISABLING 4
#define STARTING 5
#define STOP_SWITCH 6

const bool SERIAL_ON = true;
const int PROCESS_DELAY = COM_DELAY;

TaskHandle_t Main;
TaskHandle_t Communication;

ESP_FlexyStepper translation_stepper;
ESP_FlexyStepper rotation_stepper;

bool zero();
void setControl();
void CANSender();
void CANReceiver();
void evaluateState();
// void IRAM_ATTR Increment_Translation();
// void IRAM_ATTR Increment_Rotation();

int board_ID = 0;

int state = 0;
bool emergency_stop = false;

double translation_measured = 0;
double rotation_measured = 0;
double translation_desired = 0;
double rotation_desired = 0;

int translation_sensor = 0;
int rotationSensor = 0;
bool rotationSensorActive = false;
bool translationSensorActive = false;

unsigned long message_time = 0;
double start_time = 0;

typedef union {
    float value;
    byte bytes[sizeof(float)];
} FLOAT_BYTE_UNION;

void setup() {
    if (SERIAL_ON) {
        Serial.begin(115200);
    }
    
    //Pin Initialization
    pinMode(ID_1, INPUT);
    pinMode(ID_2, INPUT);
    pinMode(ENABLE, INPUT);
    // attachInterrupt(TRANSLATION_SENSOR, Increment_Translation, RISING);
    // attachInterrupt(ROTATION_SENSOR, Increment_Rotation, RISING);
    CAN.setPins(RXR_CAN, TXD_CAN);
    pinMode(ALL_GOOD_LED, OUTPUT);
    translation_stepper.connectToPins(TRANSLATION_DRIVER_PULSE, TRANSLATION_DRIVER_DIR);
    rotation_stepper.connectToPins(ROTATION_DRIVER_PULSE, ROTATION_DRIVER_DIR);

    board_ID = digitalRead(ID_2)*2 + digitalRead(ID_1);

    if (SERIAL_ON) {
        Serial.print("Board ID: ");
        Serial.println(board_ID);
    }

    translation_stepper.setStepsPerMillimeter(STEP_PULSE_TRANSLATION_CONVERSION[board_ID]);
    translation_stepper.setAccelerationInMillimetersPerSecondPerSecond(MAX_ACCELERATION_TRANSLATION);
    translation_stepper.setDecelerationInMillimetersPerSecondPerSecond(MAX_ACCELERATION_TRANSLATION);
    translation_stepper.setSpeedInMillimetersPerSecond(MAX_SPEED_TRANSLATION);

    rotation_stepper.setStepsPerRevolution(STEP_PULSE_ROTATION_CONVERSION);
    rotation_stepper.setAccelerationInRevolutionsPerSecondPerSecond(MAX_ACCELERATION_ROTATION);
    rotation_stepper.setDecelerationInRevolutionsPerSecondPerSecond(MAX_ACCELERATION_ROTATION);
    rotation_stepper.setSpeedInRevolutionsPerSecond(MAX_SPEED_ROTATION);

    while(digitalRead(ENABLE) == LOW);
    if (!zero()){
        digitalWrite(ALL_GOOD_LED, LOW);
        while(1);
    }

    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        if (SERIAL_ON) Serial.println("Starting CAN failed!");
        digitalWrite(ALL_GOOD_LED, LOW);
        while (1);
    }
    if (SERIAL_ON) Serial.println("Starting CAN success");
    CAN.filter(0b1 << board_ID, 0b10100000 + (1 << board_ID) + 0x1fffff00);
    
    message_time = millis();
    start_time = millis();
    evaluateState();
}

void loop() {
    evaluateState();
    CANReceiver();
    setControl();
    if (millis() - start_time > COM_DELAY){
        start_time = millis();
        CANSender();
    }
}


bool zero(){
    if (SERIAL_ON) Serial.print("Zeroing Translation");
    bool success_translation = zeroTrans();
    if (!success_translation) {
        if (SERIAL_ON) Serial.println(" Failed");
        return success_translation;
    }
    if (SERIAL_ON) {
        Serial.println(" Success");
        Serial.print("Zeroing Rotation");
    }
    bool success_rotation = zeroRev();
    if (!success_rotation) {
        if (SERIAL_ON) Serial.println(" Failed");
        return success_rotation;
    }
    bool success = success_rotation && success_translation;
    if (success) {
        translation_sensor = rotationSensor = 0;
    }
    if (SERIAL_ON && success) Serial.println(" Success");
    else if(SERIAL_ON) Serial.println(" Failed");
    return success;
}

bool zeroRev(){
    int timer_serial = 0;
    int timer_bounce = 0;
    int count = 0;
    bool success = false;
    pinMode(ROTATION_DRIVER_ZERO, INPUT);
    rotation_stepper.setCurrentPositionAsHomeAndStop();
    rotation_stepper.setSpeedInRevolutionsPerSecond(HOME_SPEED_ROTATION);
    double distance = 2*DIRECTIONS[board_ID][ROTATION];
    rotation_stepper.setTargetPositionInRevolutions(distance);
    while(!rotation_stepper.processMovement() && (count <= 10)){
        if (millis() - timer_bounce > 1) {
            timer_bounce = millis();
            if (digitalRead(ROTATION_DRIVER_ZERO) == LOW) {
                count += 1;
            } else {
                count = 0;
            }
        }
        if (count > 10) {
            success = true;
        }

        if (SERIAL_ON && millis() - timer_serial > 1000) {
            timer_serial = millis();
            if (digitalRead(ENABLE) == LOW) Serial.print("DISABLED");
            else Serial.print(".");
        }


        delay(1);
    }
    if (digitalRead(ROTATION_DRIVER_ZERO) == LOW) {
        rotation_stepper.setCurrentPositionAsHomeAndStop();
        return true;
    } else {
        rotation_stepper.emergencyStop();
        return false;
    }
    
}

bool zeroTrans(){
    int timer_serial = 0;
    int timer_bounce = 0;
    int count = 0;
    bool success = false;
    pinMode(TRANSLATION_DRIVER_ZERO, INPUT);
    translation_stepper.setCurrentPositionAsHomeAndStop();
    translation_stepper.setSpeedInMillimetersPerSecond(HOME_SPEED_ROTATION);
    double distance = DIRECTIONS[board_ID][TRANSLATION]*-1*MAX_TRANSLATIONS[board_ID];
    translation_stepper.setTargetPositionInMillimeters(distance);
    while(!translation_stepper.processMovement() && (count <= 20)){
        if (millis() - timer_bounce > 1) {
            timer_bounce = millis();
            if (digitalRead(TRANSLATION_DRIVER_ZERO) == LOW) {
                count += 1;
            } else {
                count = 0;
            }
        }
        if (count > 10) {
            success = true;
        }

        if (SERIAL_ON && millis() - timer_serial > 1000) {
            timer_serial = millis();
            if (digitalRead(ENABLE) == LOW) Serial.print("DISABLED");
            else Serial.print(".");
        }
        delay(1);
    }
    if (success) {
        translation_stepper.setCurrentPositionAsHomeAndStop();
        translation_stepper.setTargetPositionInMillimeters(DIRECTIONS[board_ID][TRANSLATION]);
        while(!translation_stepper.processMovement());
        translation_stepper.setCurrentPositionAsHomeAndStop();
        return true;
    } else {
        translation_stepper.emergencyStop();
        return false;
    }
}

void setControl(){
    if (state == RUNNING) {
        if (translation_desired >= MAX_TRANSLATIONS[board_ID]) translation_desired = MAX_TRANSLATIONS[board_ID] - 1;
        if (translation_desired < 0) translation_desired = 0;
        translation_stepper.setTargetPositionInMillimeters(DIRECTIONS[board_ID][TRANSLATION]*translation_desired);
        rotation_stepper.setTargetPositionInRevolutions(DIRECTIONS[board_ID][ROTATION]*rotation_desired);
    }
    // if (rotationSensorActive) rotation_stepper.setCurrentPositionInRevolutions(rotationSensor);
    // if (translationSensorActive) translation_stepper.setCurrentPositionInMillimeters(translation_sensor);
}

void CANSender(){
    FLOAT_BYTE_UNION translation_measured_f;
    FLOAT_BYTE_UNION rotation_measured_f;
    translation_measured_f.value = (float)(DIRECTIONS[board_ID][TRANSLATION]*translation_stepper.getCurrentPositionInMillimeters());
    rotation_measured_f.value = (float)(DIRECTIONS[board_ID][ROTATION]*rotation_stepper.getCurrentPositionInRevolutions()*DEGREES_PER_REVOLUTION);
    if (SERIAL_ON) Serial.print("Sent: (packet: 0b");
    if (state == RUNNING) {
        CAN.beginPacket(0b10010000 + (1 << board_ID));
        for (int i = sizeof(float)-1; i >= 0; i--){
            CAN.write(translation_measured_f.bytes[i]);
        }
        for (int i = sizeof(float)-1; i >= 0; i--){
            CAN.write(rotation_measured_f.bytes[i]);
        }
        CAN.endPacket();
        if (SERIAL_ON) Serial.print(0b10010000 + (1 << board_ID), BIN);
    }
    else if (state == DISABLED || state == EMERGENCY_STOP || state == STOP_SWITCH) {
        CAN.beginPacket((1 << board_ID) + 0b10000000);
        for (int i = sizeof(float)-1; i >= 0; i--){
            CAN.write(translation_measured_f.bytes[i]);
        }
        for (int i = sizeof(float)-1; i >= 0; i--){
            CAN.write(rotation_measured_f.bytes[i]);
        }
        CAN.endPacket();
        if (SERIAL_ON) Serial.print((1 << board_ID) + 0b10000000, BIN);
    } else {
        if (SERIAL_ON) Serial.println("NON PRINT STATE");
    }
    if (SERIAL_ON) {
        Serial.print(" ROTATION: ");
        Serial.print(rotation_measured_f.value);
        Serial.print(" Translation: ");
        Serial.println(translation_measured_f.value);
    }
}

void CANReceiver(){
    if (CAN.parsePacket()){
        message_time = millis();
        FLOAT_BYTE_UNION translation_desired_f;
        FLOAT_BYTE_UNION rotation_desired_f;
        if (SERIAL_ON) {
            Serial.print("Recv: (packet: 0b");
            Serial.print(CAN.packetId(), BIN);
            Serial.print(" ");
        }
    
        //EMERGENCY STOP
        if ((CAN.packetId() & 0b11110000) == 0b00000000) {
            emergency_stop = true;
            evaluateState();
            return;
        } else {
            emergency_stop = false;
        }
    
        //zero
        if ((CAN.packetId() & 0b11000000) == 0b01000000) {
            zero();
            return;
        }
    
        int rotation_index = sizeof(float)-1;
        int translation_index = sizeof(float)-1;
        bool data_received = false;
        while(CAN.available()){
            data_received = true;
            if (translation_index >= 0){
                translation_desired_f.bytes[translation_index--] = (byte)CAN.read();
            } else if (rotation_index >= 0) {
                rotation_desired_f.bytes[rotation_index--] = (byte)CAN.read();
            } else {
                if (SERIAL_ON) Serial.println("DATA DROPPED)");
                return;
            }
        }
        if (data_received){
            if (!((translation_index < 0) && (rotation_index < 0))) {
                if (SERIAL_ON) Serial.println("LESS THAN 8 BYTES RECEIVED)");
                return;
            }
            rotation_desired = (double)rotation_desired_f.value/DEGREES_PER_REVOLUTION;
            translation_desired = (double)translation_desired_f.value;
            if (SERIAL_ON){
                Serial.print(" Rotation: ");
                Serial.print(rotation_desired*DEGREES_PER_REVOLUTION);
                Serial.print(" Translation: ");
                Serial.print(translation_desired);
            }
        }
        if (SERIAL_ON) Serial.println(")");
    }
}

//Zeroing may mess this up
void evaluateState(){
    if (state == OFF){
        if (emergency_stop) {
            state = DISABLING;
        } else {
            state = STARTING;
        }
    } else if (state == RUNNING){
        if (emergency_stop || (millis() - message_time > MAX_COM_DELAY) || (digitalRead(ENABLE) == LOW)) {
            state = DISABLING;
        }
    } else if (state == DISABLED){
        if (emergency_stop) state = EMERGENCY_STOP;
        else if ((millis() - message_time < MAX_COM_DELAY) && (digitalRead(ENABLE) == HIGH)) {
            state = STARTING;
        }
        if ((digitalRead(ENABLE) == LOW) && SERIAL_ON) Serial.println("STOP SWITCH ACTIVE");
    } else if (state == EMERGENCY_STOP){
        if (SERIAL_ON) Serial.println("EMERGENCY_STOP");
        if (!emergency_stop) {
            state = DISABLED;
        }
    } else if (state == STOP_SWITCH) {
        if (digitalRead(ENABLE) == HIGH){
            zero();
            state = DISABLED;
        }
    }

    if (state == STARTING){
        if (SERIAL_ON) Serial.println("STARTING");
        if (emergency_stop) {
            state = DISABLING;
        } else {
            translation_stepper.startAsService(STEPPER_CORE);
            rotation_stepper.startAsService(STEPPER_CORE);
            digitalWrite(ALL_GOOD_LED, HIGH);
            state = RUNNING;
            if (SERIAL_ON) Serial.println("STARTED");
        }
    }
    if (state == DISABLING){
        if (SERIAL_ON) Serial.println("DISABLING");
        translation_stepper.emergencyStop(false);
        rotation_stepper.emergencyStop(false);
        translation_stepper.stopService();
        rotation_stepper.stopService();
        digitalWrite(ALL_GOOD_LED, LOW);
        if (emergency_stop) {
            state = EMERGENCY_STOP;
        } else if(digitalRead(ENABLE) == LOW) {
            state = STOP_SWITCH;
        } else state = DISABLED;
        if (SERIAL_ON) Serial.println("DISABLED");
    }
}

// void IRAM_ATTR Increment_Translation(){
//     int spin_direction = translation_stepper.getDirectionOfMotion();
//     if (spin_direction > 0) {
//         translation_sensor += 1 * SENSOR_PULSE_TRANSLATION_CONVERSION;
//     } else if (spin_direction < 0) {
//         translation_sensor -= 1 * SENSOR_PULSE_TRANSLATION_CONVERSION;
//     } else {
//         if (SERIAL_ON) Serial.println("Increment_Translation called with movement = 0");
//     }
//     if (!translationSensorActive && abs(translation_sensor) > MINIMUM_SENSOR_PULSES * SENSOR_PULSE_TRANSLATION_CONVERSION) {
//         if (SERIAL_ON) Serial.println("Translation Sensor Active");
//         translationSensorActive = true;
//     }
// }

// void IRAM_ATTR Increment_Rotation(){
//     int spin_direction = rotation_stepper.getDirectionOfMotion();
//     if (spin_direction > 0) {
//         rotationSensor += 1 * SENSOR_PULSE_ROTATION_CONVERSION;
//     } else if (spin_direction < 0) {
//         rotationSensor -= 1 * SENSOR_PULSE_ROTATION_CONVERSION;
//     } else {
//         if (SERIAL_ON) Serial.println("Increment_Rotation called with movement = 0");
//     }
//     if (!rotationSensorActive && abs(rotationSensor) > MINIMUM_SENSOR_PULSES * SENSOR_PULSE_ROTATION_CONVERSION) {
//         if (SERIAL_ON) Serial.println("Rotation Sensor Active");
//         rotationSensorActive = true;
//     }
// }
