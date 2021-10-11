#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

#define SERVO_PIN 5

#include <Servo.h>

class Servo0
{
  public:
    void init(int min, int max);
    void rotate(int degree);
    
  private:
    Servo servo;
    int min, max;
};
#endif // SERVO_MOTOR_H
