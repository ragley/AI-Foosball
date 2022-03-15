#define ID_1 23
#define ID_2 22
#define ROTATION_DRIVER_PULSE 18
#define ROTATION_DRIVER_DIR 5
#define ROTATION_DRIVER_ZERO 19
#define TRANSLATION_DRIVER_PULSE 17
#define TRANSLATION_DRIVER_DIR 16
#define TRANSLATION_DRIVER_ZERO 4
#define ROTATION_SENSOR  21
#define TRANSLATION_SENSOR  2
#define TXD_CAN 15
#define RXR_CAN 13
#define ENABLE 34

#define ALL_GOOD_LED 2

#define STEPPER_CORE 0
#define CONTROL_CORE 1
#define COM_CORE 1

#define ROTATION 1
#define TRANSLATION 0

const int directions[4][2] = {{-1, 1}, //3 rod
                              {-1, 1}, //5 rod
                              { 1, 1}, //2 rod
                              {-1, 1}};//Goal rod



const double maxTranslations[4] = {224, //3 rod
                                   112, //5 rod
                                   352, //2 rod
                                   180};//Goal rod

const double mmPerRevolution = 29.924;

const double STEP_PULSE_ROTATION_CONVERSION = 1600; //pulse per rotation (200 pulses on the motor with the driver set at 3200) 
const double STEP_PULSE_TRANSLATION_CONVERSION = STEP_PULSE_ROTATION_CONVERSION/mmPerRevolution; //pulse per mm 
//const double STEP_PULSE_TRANSLATION_CONVERSION = 8*1/0.474; //pulse per mm (8 corrects the resolution as above)

const double maxSpeedRotation = 89; //rotations per second
const double maxSpeedTranslation = maxSpeedRotation*mmPerRevolution; //mm per second
const double maxAccelerationRotation = 500; // rotations per second per second
const double maxAccelerationTranslation = maxAccelerationRotation*mmPerRevolution; // mm per second per second
const double homeSpeedTranslation = maxSpeedTranslation / 10;
const double homeSpeedRotation = maxSpeedRotation / 10;

const int COM_DELAY = 20; //in ms
const int MAX_COM_DELAY = COM_DELAY * 5;

const double SENSOR_PULSE_TRANSLATION_CONVERSION = 0;
const double SENSOR_PULSE_ROTATION_CONVERSION = 0;

const int MINIMUM_SENSOR_PULSES = 10;
