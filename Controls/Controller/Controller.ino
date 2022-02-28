#include <CAN.h>
#include <ESP_FlexyStepper.h>

const bool SERIAL_ON = true;

#define ID_2 23
#define ID_1 22
#define ROTATION_DRIVER_PULSE 21
#define ROTATION_DRIVER_DIR 19
#define ROTATION_DRIVER_ZERO 18
#define TRANSLATION_DRIVER_PULSE 5
#define TRANSLATION_DRIVER_DIR 17
#define TRANSLATION_DRIVER_ZERO 16
#define ROTATION_ENCODER  4
#define TRANSLATION_ENCODER  2
#define TXD_CAN 15
#define RXR_CAN 13
#define ENABLE 34

#define ALL_GOOD_LED 2

#define STEPPER_CORE 0
#define CONTROL_CORE 0
#define COM_CORE 1

TaskHandle_t Controller;
TaskHandle_t Communication;

ESP_FlexyStepper Translation_Stepper;
ESP_FlexyStepper Rotation_Stepper;

const double maxTranslations = [1,1,1,1];

const double maxSpeedTranslation = 0; //mm per second
const double maxSpeedRotation = 0; //rotations per second
const double maxAccelerationTranslation = 0; // mm per second per second
const double maxAccelerationRotation = 0; // rotations per second per second
const double homeSpeedTranslation =  maxSpeedTranslation / 10;
const double homeSpeedRotation =  maxSpeedRotation / 10;
const signed char homeDirTranslation = 1;
const signed char homeDirRotation = 1;

const int COM_DELAY = 2000; //in ms
const int MAX_COM_DELAY = COM_DELAY * 3;

const double STEP_PULSE_TRANSLATION_CONVERSION = 1/0.474; //pulse per mm
const double STEP_PULSE_ROTATION_CONVERSION = 200; //pulse per rotation

const double ENCODER_PULSE_TRANSLATION_CONVERSION = 0;
const double ENCODER_PULSE_ROTATION_CONVERSION = 0;

void SHUTDOWN();
void START();
bool CAN_running();
void Controller_Core(void * parameters);
void CAN_Sender(void * parameters);
void CAN_Handler(int packet_size);
void IRAM_ATTR Increment_Displacement();
void IRAM_ATTR Increment_Rotation();

int ID = 0;

double translationMeasured = 0;
double rotationMeasured = 0;
double translationDesired = 0;
double rotationDesired = 0;

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

unsigned long messageTime = 0;
bool RUNNING = true;

bool E_STOP = false;

void setup() {
    if (SERIAL_ON) {
        Serial.begin(115200);
        while (!Serial);
    }
    
    //Pin Initialization
    pinMode(ID_2, INPUT);
    pinMode(ID_1, INPUT);

    ID = digitalRead(ID_2)*2 + digitalRead(ID_1);

    ESP_FlexyStepper Translation_Stepper;
    ESP_FlexyStepper Rotation_Stepper;

    Translation_Stepper.connectToPins(TRANSLATION_DRIVER_PULSE, TRANSLATION_DRIVER_DIR);
    Translation_Stepper.setStepsPerMillimeter(STEP_PULSE_TRANSLATION_CONVERSION);
    Translation_Stepper.setAccelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);
    Translation_Stepper.setDecelerationInMillimetersPerSecondPerSecond(maxAccelerationTranslation);
    Translation_Stepper.setSpeedInMillimetersPerSecond(maxSpeedTranslation);

    Rotation_Stepper.connectToPins(ROTATION_DRIVER_PULSE, ROTATION_DRIVER_DIR);
    Rotation_Stepper.setStepsPerMillimeter(STEP_PULSE_ROTATION_CONVERSION);
    Rotation_Stepper.setAccelerationInMillimetersPerSecondPerSecond(maxAccelerationRotation);
    Rotation_Stepper.setDecelerationInMillimetersPerSecondPerSecond(maxAccelerationRotation);
    Rotation_Stepper.setSpeedInMillimetersPerSecond(maxSpeedRotation);

    attachInterrupt(TRANSLATION_ENCODER, Increment_Displacement, RISING);
    attachInterrupt(ROTATION_ENCODER, Increment_Rotation, RISING);

    CAN.setPins(RXR_CAN, TXD_CAN);

    pinMode(ALL_GOOD_LED, OUTPUT);

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

    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        if (SERIAL_ON) Serial.println("Starting CAN failed!");
        digitalWrite(ALL_GOOD_LED, LOW);
        while (1);
    }
    if (SERIAL_ON) Serial.println("Starting CAN success");
    CAN.filter(0b011111, (1 << ID) + (0b100000));
    CAN.onReceive(CAN_Handler);
    if ~(ZERO()){
        digitalWrite(ALL_GOOD_LED, LOW);
        while(1);
    }
    messageTime = millis();
    START();
}

void loop() {
}

bool ZERO(){
    return 
    (Translation_Stepper.moveToHomeInMillimeters(homeDirTranslation, homeSpeedTranslation, maxTranslations[ID], TRANSLATION_DRIVER_ZERO) 
    &&
    Rotation_Stepper.moveToHomeInRevolutions(homeDirRotation, homeSpeedRotation, 1, ROTATION_DRIVER_ZERO));
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
            if (displacementPID.Compute()){
                //Directional Control
                if (PWM_DISPLACEMENT_CH > 0) {
                    digitalWrite(DISPLACEMENT_DRIVER_1, HIGH);
                    digitalWrite(DISPLACEMENT_DRIVER_2, LOW);
                } else {
                    digitalWrite(DISPLACEMENT_DRIVER_1, LOW);
                    digitalWrite(DISPLACEMENT_DRIVER_2, HIGH);
                }
                //ledcWrite(PWM_DISPLACEMENT_CH, abs(displacementPWMFrequency));
                ledc_set_freq(SPEED_MODE , DISPLACEMENT_TIMER, abs(displacementPWMFrequency));
            }
            if (rotationPID.Compute()) {
                //Directional Control
                if (PWM_ROTATION_CH > 0) {
                    digitalWrite(ROTATION_DRIVER_1, HIGH);
                    digitalWrite(ROTATION_DRIVER_2, LOW);
                } else {
                    digitalWrite(ROTATION_DRIVER_1, LOW);
                    digitalWrite(ROTATION_DRIVER_2, HIGH);
                }
                //ledcWrite(PWM_ROTATION_CH, abs(rotationPWMFrequency));
                ledc_set_freq(SPEED_MODE, ROTATION_TIMER, abs(rotationPWMFrequency));
            }
        }
    }
}

//Need to decide on packet format
void CAN_Sender(void * parameters){
    for(;;){
        displacementMeasured_f.value = (float)displacementMeasured;
        rotationMeasured_f.value = (float)rotationMeasured;
        if (CAN_running()) {
            CAN.beginPacket(1 << ID);
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(displacementMeasured_f.bytes[i]);
            }
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(rotationMeasured_f.bytes[i]);
            }
            CAN.endPacket();
        }
        if (!RUNNING) {
            CAN.beginPacket(1 << ID + 0b010000);
            for (int i = 0; i < sizeof(float); i++){
                CAN.write(displacementMeasured_f.bytes[i]);
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
    //EMERGENCY STOP
    if (CAN.packetId() & 0b010000 == 0b010000) {
        SHUTDOWN();
        E_STOP = true;
        return;
    }

    messageTime = millis();


    if (SERIAL_ON) {
        Serial.print("CAN_Handler core: ");
        Serial.print("");
        // received a packet
        Serial.print("Received ");

        if (CAN.packetExtended()) {
            Serial.print("extended ");
        }

        if (CAN.packetRtr()) {
            // Remote transmission request, packet contains no data
            Serial.print("RTR ");
        }

        Serial.print("packet with id 0x");
        Serial.print(CAN.packetId(), HEX);

        if (CAN.packetRtr()) {
            Serial.print(" and requested length ");
            Serial.println(CAN.packetDlc());
        } else {
            Serial.print(" and length ");
            Serial.println(packet_size);

            // only print packet data for non-RTR packets
            while (CAN.available()) {
            Serial.print((char)CAN.read());
            }
            Serial.println();
        }

        Serial.println();
    }
    if (!RUNNING) START();
}

void IRAM_ATTR Increment_Displacement(){
    if (displacementPWMFrequency > 0) {
        displacementMeasured += 1 * PULSE_DISPLACEMENT_CONVERSION;
    } else if (displacementPWMFrequency < 0) {
        displacementMeasured -= 1 * PULSE_DISPLACEMENT_CONVERSION;
    } else {
        if (SERIAL_ON) Serial.println("Increment_Displacement called with PWM = 0");
    }
}

void IRAM_ATTR Increment_Rotation(){
    if (rotationPWMFrequency > 0) {
        rotationMeasured += 1 * PULSE_ROTATION_CONVERSION;
    } else if (rotationPWMFrequency < 0) {
        rotationMeasured -= 1 * PULSE_ROTATION_CONVERSION;
    } else {
        if (SERIAL_ON) Serial.println("Increment_Rotation called with PWM = 0");
    }
}
