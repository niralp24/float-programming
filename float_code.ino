#include <Arduino.h>
#include <ESP32Time.h>


ESP32Time rtc;

const int depthsensorPin = 25;
const int lightsensorpin = 15;
const int lightresistorohm = 10000; 
const int thermistorpin = 26;
const int thermistorresistorohm = 10000;

//--- THIS CODE SHOULD BE COMMENTED IF RUNNING IT WITH ESP32_WiFi---
//void setup() {
//    Serial.begin(9600);
//}


//void loop() {
    //get_depth();
    //get_temp();
    //get_lux();

//}
float get_depth(){
    int rawValue = analogRead(depthsensorPin);
    float sensorVoltage = (rawValue / 4095.0) * 3.3;
    float pressurePSI = ((sensorVoltage - 0.5) / 4.0) * 10.0;
    if (pressurePSI < 0) {
        pressurePSI = 0;
    }
    float pressurePa = pressurePSI * 6894.76;
    float depth = pressurePa / (1000.0 * 9.81);
    return depth;
}

float get_temp(){
    int rawThermistorValue = analogRead(thermistorpin);
    float thermistorVoltage = (rawThermistorValue / 4095.0);

    return thermistorVoltage;
}

float get_lux(){
        // Calculate the light sensor voltage, current, and lux level
    int rawlightValue = analogRead(lightsensorpin);
    float lightSensorVoltage = (rawlightValue / 4095.0) * 3.3;
    float lightSensorAmps = lightSensorVoltage / lightresistorohm;
    float lightSensorMicroAmps = lightSensorAmps * 1e6;
    float luxlevel = lightSensorMicroAmps / 3.5; 
    return luxlevel;
}
