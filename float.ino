#include <Arduino.h>

const int sensor_pin = 25;

void setup() {
    Serial.begin(115200)

}


void loop() {
    int rawValue = analogRead(sensor_pin)
    float adcVoltage = (rawValue / 4095.0) * 3.3
}
// Ratio of Pressure Sensor to PSI Equation; y = 0.4x+0.5
// 1 PSI is 0.703 meters