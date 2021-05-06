#include <stdio.h>
#include <sys/select.h>
#include <time.h>

#include "rpiplc-arduino.h"
#include "rpiplc-peripherals.h"

#include "ads1015.h"
#include "mcp23008.h"
#include "pca9685.h"

static i2c_t i2c;

////////////////////////////////////////////////////////////////////////////////////////////////////
static int isAddressIntoArray(uint8_t addr, const uint8_t* arr, uint8_t len) {
	while (len--) {
		if (*arr++ == addr) {
			return 1;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void initPins() {
	i2c_init(&i2c, I2C_BUS);

	for (int i = 0; i < rpiplc_num_pca9685; ++i) {
		if (!pca9685_init(&i2c, rpiplc_pca9685[i])) {
			fprintf(stderr, "initPins: init PCA9685 (%02x) error\n", rpiplc_pca9685[i]);
		}
	}

	for (int i = 0; i < rpiplc_num_ads1015; ++i) {
		if (!ads1015_init(&i2c, rpiplc_ads1015[i])) {
			fprintf(stderr, "initPins, init ADS1015 (%02x) error\n", rpiplc_ads1015[i]);
		}
	}

	for (int i = 0; i < rpiplc_num_mcp23008; ++i) {
		if (!mcp23008_init(&i2c, rpiplc_mcp23008[i])) {
			fprintf(stderr, "initPins, init MCP23008 (%02x) error\n", rpiplc_mcp23008[i]);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void pinMode(uint32_t pin, uint8_t mode) {
	uint8_t addr = pinToDeviceAddress(pin);
	uint8_t index = pinToDeviceIndex(pin);

	if (isAddressIntoArray(addr, rpiplc_mcp23008, rpiplc_num_mcp23008)) {
		if (mode == INPUT) {
			mcp23008_set_pin_mode(&i2c, addr, index, MCP23008_INPUT);
		} else if (mode == OUTPUT) {
			mcp23008_set_pin_mode(&i2c, addr, index, MCP23008_OUTPUT);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void digitalWrite(uint32_t pin, int value) {
	uint8_t addr = pinToDeviceAddress(pin);
	uint8_t index = pinToDeviceIndex(pin);

	if (isAddressIntoArray(addr, rpiplc_pca9685, rpiplc_num_pca9685)) {
		if (value) {
			if (!pca9685_set_out_on(&i2c, addr, index)) {
				fprintf(stderr, "digitalWrite: set PCA9685 error\n");
			}
		} else {
			if (!pca9685_set_out_off(&i2c, addr, index)) {
				fprintf(stderr, "digitalWrite: set PCA9685 error\n");
			}
		}

	} else if (isAddressIntoArray(addr, rpiplc_mcp23008, rpiplc_num_mcp23008)) {
		if (!mcp23008_write(&i2c, addr, index, value)) {
			fprintf(stderr, "digitalWrite: set MCP23008 error\n");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int digitalRead(uint32_t pin) {
	uint8_t addr = pinToDeviceAddress(pin);
	uint8_t index = pinToDeviceIndex(pin);

	if (isAddressIntoArray(addr, rpiplc_mcp23008, rpiplc_num_mcp23008)) {
		return mcp23008_read(&i2c, addr, index);
	} else if (isAddressIntoArray(addr, rpiplc_ads1015, rpiplc_num_ads1015)) {
		return ads1015_read(&i2c, addr, index) > 1023 ? 1 : 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void analogWrite(uint32_t pin, int value) {
	uint8_t addr = pinToDeviceAddress(pin);
	uint8_t index = pinToDeviceIndex(pin);

	if (isAddressIntoArray(addr, rpiplc_pca9685, rpiplc_num_pca9685)) {
		if (!pca9685_set_out_pwm(&i2c, addr, index, value)) {
			fprintf(stderr, "analogWrite: set PCA9685 error\n");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t analogRead(uint32_t pin) {
	uint8_t addr = pinToDeviceAddress(pin);
	uint8_t index = pinToDeviceIndex(pin);

	if (isAddressIntoArray(addr, rpiplc_ads1015, rpiplc_num_ads1015)) {
		return ads1015_read(&i2c, addr, index);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void delay(uint32_t milliseconds) {
	struct timeval tv = {
		.tv_sec = (long) (milliseconds / 1000),
		.tv_usec = (long) (milliseconds % 1000) * 1000L,
	};
	select(0, NULL, NULL, NULL, &tv);
}