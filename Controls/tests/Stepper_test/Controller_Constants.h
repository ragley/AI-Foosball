#define ID_2 23
#define ID_1 22
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

const double maxTranslations[4] = {224, //3 rod
                                   112, //5 rod
                                   352, //2 rod
                                   180};//Goal rod

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

const int MINIMUM_ENCODER_PULSES = 10;