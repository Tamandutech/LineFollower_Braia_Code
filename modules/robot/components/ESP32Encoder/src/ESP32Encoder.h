#pragma once
#include <stdbool.h>
#include <driver/gpio.h>
#include "driver/pulse_cnt.h"
#include "esp_attr.h"
#include "esp_log.h"


enum encType {
single,
half,
full
};

class ESP32Encoder {
private:
	void attach(int aPintNumber, int bPinNumber, enum encType et);
	bool attached=false;
    bool direction;
    bool working;
public:
	ESP32Encoder();
	~ESP32Encoder();
	void attachHalfQuad(int aPintNumber, int bPinNumber);
	void attachFullQuad(int aPintNumber, int bPinNumber);
	void attachSingleEdge(int aPintNumber, int bPinNumber);
	//void attachHalfQuad(int aPintNumber, int bPinNumber);
	int32_t getCount();
	int32_t getCountRaw();
	int32_t clearCount();
	int32_t pauseCount();
	int32_t resumeCount();

	bool isAttached(){return attached;}
	void setCount(int32_t value);
	static bool attachedInterrupt;
	gpio_num_t aPinNumber;
	gpio_num_t bPinNumber;
	pcnt_unit_config_t unit;
	pcnt_unit_handle_t pcnt_unit = NULL;
	bool fullQuad=false;
	int countsMode = 2;
	volatile int32_t count=0;
	pcnt_chan_config_t channA;
	pcnt_chan_config_t channB;
	static bool useInternalWeakPullResistors;
};
