#ifndef ACS712_h
#define ACS712_h

#include <Arduino.h>

#define ADC_SCALE 1023.0


enum ACS712_type {ACS712_05B, ACS712_20A, ACS712_30A};

class ACS712 {
public:
	ACS712(ACS712_type type, uint8_t _pin);
	int calibrate();
	void setZeroPoint(int _zero);
	float getCurrentDC();
	float getCurrentAC50Hz();
	float getCurrentAC60Hz();
    float getCurrentAC(uint16_t frequency);
    long readVcc();

private:
	float zero = 512.0;
    float vref = 5;
	float sensitivity;
	float typeToSensitivity(ACS712_type type);
	uint8_t pin;
};

#endif