#include "ACS712.h"

ACS712::ACS712(ACS712_type type, uint8_t _pin) {
    sensitivity = typeToSensitivity(type);
	pin = _pin;
}

float ACS712::typeToSensitivity(ACS712_type type) {
 	switch (type) {
		case ACS712_05B:
			return 0.185;
		case ACS712_20A:
			return 0.100;
		case ACS712_30A:
			return 0.066;
		default:
			return 0.066;
    }
}

int ACS712::calibrate() {
	int _zero = 0;
	for (int i = 0; i < 10; i++) {
		_zero += analogRead(pin);
		delay(10);
	}
	_zero /= 10;
	zero = _zero;
	return _zero;
}

void ACS712::setZeroPoint(int _zero) {
	zero = _zero;
}

float ACS712::getCurrentDC() {
	float I = (zero - analogRead(pin)) / ADC_SCALE * (readVcc() / 1000.0) / sensitivity;
	return I;
}

float ACS712::getCurrentAC60Hz() {
	return getCurrentAC(60);
}

float ACS712::getCurrentAC50Hz() {
	return getCurrentAC(50);
}

float ACS712::getCurrentAC(uint16_t frequency) {
	uint32_t period = 1000000 / frequency;
	uint32_t t_start = micros();

	uint32_t Isum = 0, measurements_count = 0;
	int32_t Inow;

	while (micros() - t_start < period) {
		Inow = zero - analogRead(pin);
		Isum += Inow*Inow;
		measurements_count++;
	}

	float Irms = sqrt(Isum / measurements_count) / ADC_SCALE * (readVcc() / 1000.0) / sensitivity;
	return Irms;
}

long ACS712::readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif
  
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  
  uint8_t low  = ADCL; // must read ADCL first – it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  
  long result = (high << 8) | low;
  
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
