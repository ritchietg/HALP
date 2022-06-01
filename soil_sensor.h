#ifndef SOIL_SENSOR
#define SOIL_SENSOR

class SoilSensor {
  private:
    int pin = -1;
    int minRate = -1;
    int maxRate = -1;
    int minimum = -1;
    int maximum = -1;

  public:
    int rate = -1;
    int formattedRate = -1;
    SoilSensor(int sensorPin, int mi, int ma);
    int getRate(int count, int wait);
};

#endif
