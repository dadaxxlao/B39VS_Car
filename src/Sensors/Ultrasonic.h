#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

class Ultrasonic {
private:
    int trigPin;
    int echoPin;
    
public:
    Ultrasonic(int trig = 2, int echo = 3) : trigPin(trig), echoPin(echo) {}
    
    void init() {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }
    
    float getDistance() {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        
        float duration = pulseIn(echoPin, HIGH);
        return duration * 0.034 / 2; // 转换为厘米
    }
};

#endif // ULTRASONIC_H 