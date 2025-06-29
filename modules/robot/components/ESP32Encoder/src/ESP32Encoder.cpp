/*
 * ESP32Encoder.cpp
 *
 *  Created on: Oct 15, 2018
 *      Author: hephaestus
 */

#include <ESP32Encoder.h>

// static ESP32Encoder *gpio2enc[48];
//
//
bool ESP32Encoder::useInternalWeakPullResistors = true;

ESP32Encoder::ESP32Encoder()
{
	attached = false;
	aPinNumber = (gpio_num_t)0;
	bPinNumber = (gpio_num_t)0;
	working = false;
	direction = false;
}

ESP32Encoder::~ESP32Encoder()
{
	// TODO Auto-generated destructor stub
}

void ESP32Encoder::attach(int a, int b, enum encType et)
{
	if (attached)
	{
		ESP_LOGE("ESP32Encoder", "All ready attached, FAIL!");
		return;
	}

	// Set data now that pin attach checks are done
	fullQuad = et != single;
	unit = {
        .low_limit = INT16_MIN,
        .high_limit = INT16_MAX,
    };
	unit.flags.accum_count = 1;
	this->aPinNumber = (gpio_num_t)a;
	this->bPinNumber = (gpio_num_t)b;

	ESP_ERROR_CHECK(pcnt_new_unit(&unit, &pcnt_unit));

	ESP_LOGI("ESP32Encoder", "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

	// Set up the IO state of hte pin
		gpio_set_direction(aPinNumber, GPIO_MODE_INPUT);
		gpio_set_direction(bPinNumber, GPIO_MODE_INPUT);
		if (useInternalWeakPullResistors)
		{
			gpio_pulldown_en(aPinNumber);
			gpio_pulldown_en(bPinNumber);
		}

    ESP_LOGI("ESP32Encoder", "install pcnt channels");
    channA = {
        .edge_gpio_num = this->aPinNumber,
        .level_gpio_num = this->bPinNumber,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &channA, &pcnt_chan_a));
    
	pcnt_chan_config_t channB = {
        .edge_gpio_num = this->bPinNumber,
        .level_gpio_num = this->aPinNumber,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &channB, &pcnt_chan_b));

	// Set up encoder PCNT configuration
	ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, fullQuad ? PCNT_CHANNEL_EDGE_ACTION_DECREASE:PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_INVERSE, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

	if (et == full)
	{
		// set up second channel for full quad
		ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    	ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));	
	}
	else
	{// make sure channel 1 is not set when not full quad
		ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    	ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_HOLD, PCNT_CHANNEL_LEVEL_ACTION_HOLD));
	}

	/* Enable events on  maximum and minimum limit values */
	ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, INT16_MAX));
	ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, INT16_MIN));


  	ESP_LOGI("ESP32Encoder", "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_LOGI("ESP32Encoder", "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_LOGI("ESP32Encoder", "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

}

void ESP32Encoder::attachHalfQuad(int aPintNumber, int bPinNumber)
{
	attach(aPintNumber, bPinNumber, half);
}
void ESP32Encoder::attachSingleEdge(int aPintNumber, int bPinNumber)
{
	attach(aPintNumber, bPinNumber, single);
}
void ESP32Encoder::attachFullQuad(int aPintNumber, int bPinNumber)
{
	attach(aPintNumber, bPinNumber, full);
}

void ESP32Encoder::setCount(int32_t value)
{
	count = value - getCountRaw();
}
int32_t ESP32Encoder::getCountRaw()
{
	int c;
	ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &c));
	return c;
}
int32_t ESP32Encoder::getCount()
{
	return getCountRaw() + count;
}

int32_t ESP32Encoder::clearCount()
{
	count = 0;
	ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
	return ESP_OK;
}

int32_t ESP32Encoder::pauseCount()
{
	return pcnt_unit_stop(pcnt_unit);
}

int32_t ESP32Encoder::resumeCount()
{
	return pcnt_unit_start(pcnt_unit);
}
