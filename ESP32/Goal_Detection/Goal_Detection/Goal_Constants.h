#define TXD_CAN 15
#define RXR_CAN 13
#define PLAYER_SENSOR 18
#define ROBOT_SENSOR 17

#define ALL_GOOD_LED 2

#define SENSOR_CORE 0

class Digital_Sensor {
  public:
    Digital_Sensor(int pin, int count){
      READ = false;
      Pressed = false;
      PIN = pin;
      COUNT = count;
      highCounter = 0;
      lowCounter = 0;
    }
    bool readSensor(){
      if (READ) {
        return false;
      } 
      if (Pressed) READ = true;
      return Pressed;
    }

    bool sensorActive(){
      return Pressed;
    }

    void sensorMonitor(){
      if (digitalRead(PIN) == HIGH){
        lowCounter = 0;
        highCounter += 1;
      } else {
        highCounter = 0;
        lowCounter += 1;
      }
      if (highCounter > COUNT) {
        Pressed = true;
      }
      if (lowCounter > COUNT) {
        READ = false;
        Pressed = false;
      }
    }

  private:
    bool READ;
    bool Pressed;
    int COUNT;
    int PIN;
    int highCounter;
    int lowCounter;
};
