#include "servo_motor.h"
#include "arduino.h"

void Servo0::init(int min, int max)
{
  this->min = min;
  this->max = max;
  rotate(90);
}

void Servo0::rotate(int degree)
{
  servo.attach(SERVO_PIN);
  degree = min >= degree ? min
          :max <= degree ? max : degree;

  servo.write(degree);
  delay(1500);
  servo.detach();
}
