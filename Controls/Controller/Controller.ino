#include <CAN.h>
#include <PID_v1.h>
#include "driver/ledc.h"

/* The next thing that needs to be done is to write
   the algorithm that will take in the physical values
   and then give the distance to the closest player */

const bool SERIAL_ON = true;

#define ID_2 23
#define ID_1 22
#define DISPLACEMENT_DRIVER_1 21
#define DISPLACEMENT_DRIVER_2 19
#define DISPLACEMENT_DRIVER_E 18
#define ROTATION_DRIVER_1  5
#define ROTATION_DRIVER_2 17
#define ROTATION_DRIVER_E 16
#define ENCODER_2  4
#define ENCODER_1  2
#define TXD_CAN 15
#define RXR_CAN 13

#define ALL_GOOD_LED 2

#define PWM_DISPLACEMENT_CH 0
#define PWM_ROTATION_CH 1

#define CONTROL_CORE 0
#define COM_CORE 1

TaskHandle_t Controller;
TaskHandle_t Communication;

const int COM_DELAY = 2000; //in ms
const int MAX_COM_DELAY = COM_DELAY * 3;

const int PWM_FREQ = 0;

const int PWM_RESOLUTION = 8;
const int PWM_DUTY_CYCLE = (2^PWM_RESOLUTION)/2;

const ledc_mode_t SPEED_MODE = LEDC_HIGH_SPEED_MODE;

const ledc_timer_t DISPLACEMENT_TIMER = LEDC_TIMER_0;
const ledc_timer_t ROTATION_TIMER = LEDC_TIMER_1;

const int PID_MAX = 255;
const int PID_MIN = -255;

const double PID_PERIOD = 0.1; //in ms

const double DISPLACEMENT_Kp = 1;
const double DISPLACEMENT_Ki = 1;
const double DISPLACEMENT_Kd = 1;

const double ROTATION_Kp = 1;
const double ROTATION_Ki = 1;
const double ROTATION_Kd = 1;

const double PULSE_DISPLACEMENT_CONVERSION = 1;
const double PULSE_ROTATION_CONVERSION = 1;

void SHUTDOWN();
void START();
bool CAN_running();
void Controller_Core(void * parameters);
void CAN_Sender(void * parameters);
void CAN_Handler(int packet_size);
void IRAM_ATTR Increment_Displacement();
void IRAM_ATTR Increment_Rotation();

int ID = 0;

double displacementMeasured = 0;
double rotationMeasured = 0;
double displacementDesired = 0;
double rotationDesired = 0;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} displacementMeasured_f;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} rotationMeasured_f;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} displacementDesired_f;

union {
    byte bytes[sizeof(float)];
    float value = 0;
} rotationDesired_f;

double displacementPWMFrequency = 0;
double rotationPWMFrequency = 0;

unsigned long messageTime = 0;
bool RUNNING = true;

bool E_STOP = false;

PID displacementPID(&displacementMeasured,
                    &displacementPWMFrequency, 
                    &displacementDesired, 
                    DISPLACEMENT_Kp,
                    DISPLACEMENT_Ki,
                    DISPLACEMENT_Kd,
                    DIRECT);

PID rotationPID(&rotationMeasured,
                    &rotationPWMFrequency, 
                    &rotationDesired, 
                    ROTATION_Kp, 
                    ROTATION_Ki, 
                    ROTATION_Kd, 
                    DIRECT);

void setup() {
    if (SERIAL_ON) {
        Serial.begin(115200);
        while (!Serial);
    }
    
    //Pin Initialization
    pinMode(ID_2, INPUT);
    pinMode(ID_1, INPUT);

    ID = digitalRead(ID_2)*2 + digitalRead(ID_1);

    pinMode(DISPLACEMENT_DRIVER_1, OUTPUT);
    pinMode(DISPLACEMENT_DRIVER_2, OUTPUT);
    ledcSetup(PWM_DISPLACEMENT_CH, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(DISPLACEMENT_DRIVER_E, PWM_DISPLACEMENT_CH);
    ledc_bind_channel_timer(SPEED_MODE, PWM_DISPLACEMENT_CH, DISPLACEMENT_TIMER);

    pinMode(ROTATION_DRIVER_1, OUTPUT);
    pinMode(ROTATION_DRIVER_2, OUTPUT);
    ledcSetup(PWM_ROTATION_CH, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(ROTATION_DRIVER_E, PWM_ROTATION_CH);
    ledc_bind_channel_timer(SPEED_MODE, PWM_ROTATION_CH, ROTATION_TIMER);

    attachInterrupt(ENCODER_1, Increment_Displacement, RISING);
    attachInterrupt(ENCODER_2, Increment_Rotation, RISING);

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

    //PID initialization
    displacementPID.SetSampleTime(PID_PERIOD);
    rotationPID.SetSampleTime(PID_PERIOD);
    displacementPID.SetOutputLimits(PID_MIN, PID_MIN);
    rotationPID.SetOutputLimits(PID_MIN, PID_MIN);

    

    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        if (SERIAL_ON) Serial.println("Starting CAN failed!");
        while (1);
    }
    if (SERIAL_ON) Serial.println("Starting CAN success");
    CAN.filter(0b011111, (1 << ID) + (0b100000));
    CAN.onReceive(CAN_Handler);

    messageTime = millis();
}

void loop() {
}

void SHUTDOWN() {
    if (RUNNING || displacementPWMFrequency != 0 || rotationPWMFrequency != 0) {
        // Disable PID
        displacementPID.SetMode(MANUAL);
        rotationPID.SetMode(MANUAL);

        //Stop motors
        displacementPWMFrequency = 0;
        rotationPWMFrequency = 0;

        digitalWrite(ALL_GOOD_LED, LOW);
        RUNNING = false;

        if (SERIAL_ON) Serial.println("SHUTDOWN");
    }
}

void START() {
    if (!RUNNING) {
        //Activating PID
        displacementPID.SetMode(AUTOMATIC);
        rotationPID.SetMode(AUTOMATIC);

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
