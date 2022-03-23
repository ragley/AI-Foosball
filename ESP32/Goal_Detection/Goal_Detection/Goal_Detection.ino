#include <CAN.h>
#include "Goal_Constants.h"

const bool SERIAL_ON = true;
const int COM_DELAY = 50;

void IRAM_ATTR playerGoalCount();
void IRAM_ATTR robotGoalCount();
void CANSender();
void CANReceiver();

typedef union {
    unsigned long value;
    byte bytes[sizeof(unsigned long)];
} LONG_BYTE_UNION;

unsigned long player_goals = 0;
unsigned long robot_goals = 0;

void setup() {
    if (SERIAL_ON) {
        Serial.begin(115200);
        while (!Serial);
    }

    attachInterrupt(PLAYER_SENSOR, playerGoalCount, RISING);
    attachInterrupt(ROBOT_SENSOR, robotGoalCount, RISING);

    CAN.setPins(RXR_CAN, TXD_CAN);

    if (!CAN.begin(500E3)) {
        if (SERIAL_ON) Serial.println("Starting CAN failed!");
        digitalWrite(ALL_GOOD_LED, LOW);
        while (1);
    }
    if (SERIAL_ON) Serial.println("Starting CAN success");
    CAN.filter(0b00100000, 0x1ffffff0);
}

double start_time = millis();
void loop() {
    CANReceiver();
    if (millis() - start_time > COM_DELAY){
        start_time = millis();
        CANSender();
    }
}

void CANSender(){
    LONG_BYTE_UNION player_goals_l;
    LONG_BYTE_UNION robot_goals_l;
    player_goals_l.value = player_goals;
    robot_goals_l.value = robot_goals;
    CAN.beginPacket(0b10110000);
    for (int i = sizeof(unsigned long)-1; i >= 0; i--){
        CAN.write(player_goals_l.bytes[i]);
    }
    for (int i = sizeof(unsigned long)-1; i >= 0; i--){
        CAN.write(robot_goals_l.bytes[i]);
    }
    CAN.endPacket();
    if (SERIAL_ON) {
        Serial.print("Sent: (packet: 0b");
        Serial.print(0b10110000, BIN);
        Serial.print(" Player Goal Count: ");
        Serial.print(player_goals_l.value);
        Serial.print(" Robot Goal Count: ");
        Serial.println(robot_goals_l.value);
    }
}

void CANReceiver(){
    if (CAN.parsePacket()){
        player_goals = 0;
        robot_goals = 0;
        if (SERIAL_ON) Serial.println("GOAL COUNT RESET");
    }
}

void IRAM_ATTR playerGoalCount(){
    player_goals += 1;
}

void IRAM_ATTR robotGoalCount(){
    robot_goals += 1;
}