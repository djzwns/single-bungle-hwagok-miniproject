#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define ECHO_PIN 3
#define TRIG_PIN 4

class Ultrasonic
{
  public:
    void init();
    float getDistance();

  private:
};

#endif // ULTRASONIC_H
