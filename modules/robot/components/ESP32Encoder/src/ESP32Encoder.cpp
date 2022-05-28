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
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = {NULL, NULL, NULL,
															NULL,
															NULL, NULL, NULL, NULL};

bool ESP32Encoder::attachedInterrupt = false;
pcnt_isr_handle_t ESP32Encoder::user_isr_handle = NULL;

ESP32Encoder::ESP32Encoder()
{
	attached = false;
	aPinNumber = (gpio_num_t)0;
	bPinNumber = (gpio_num_t)0;
	working = false;
	direction = false;
	unit = (pcnt_unit_t)-1;
}

ESP32Encoder::~ESP32Encoder()
{
	// TODO Auto-generated destructor stub
}

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
{
	ESP32Encoder *ptr;

	uint32_t intr_status = PCNT.int_st.val;
	int i;

	for (i = 0; i < PCNT_UNIT_MAX; i++)
	{
		if (intr_status & (BIT(i)))
		{
			ptr = ESP32Encoder::encoders[i];
			/* Save the PCNT event type that caused an interrupt
			 to pass it to the main program */

			int status = 0;
			if (PCNT.status_unit[i].h_lim_lat)
			{
				status = ptr->r_enc_config.counter_h_lim;
			}
			if (PCNT.status_unit[i].l_lim_lat)
			{
				status = ptr->r_enc_config.counter_l_lim;
			}
			// pcnt_counter_clear(ptr->unit);
			PCNT.int_clr.val = BIT(i); // clear the interrupt
			ptr->count = status + ptr->count;
		}
	}
}

void ESP32Encoder::attach(int a, int b, enum encType et)
{
	if (attached)
	{
		ESP_LOGE("ESP32Encoder", "All ready attached, FAIL!");
		return;
	}
	int index = 0;
	for (; index < MAX_ESP32_ENCODERS; index++)
	{
		if (ESP32Encoder::encoders[index] == NULL)
		{
			encoders[index] = this;
			break;
		}
	}
	if (index == MAX_ESP32_ENCODERS)
	{
		ESP_LOGE("ESP32Encoder", "Too many encoders, FAIL!");
		return;
	}

	// Set data now that pin attach checks are done
	fullQuad = et != single;
	unit = (pcnt_unit_t)index;
	this->aPinNumber = (gpio_num_t)a;
	this->bPinNumber = (gpio_num_t)b;

// Set up the IO state of hte pin
#ifndef ESP32_QEMU
	gpio_pad_select_gpio(aPinNumber);
	gpio_pad_select_gpio(bPinNumber);
	gpio_set_direction(aPinNumber, GPIO_MODE_INPUT);
	gpio_set_direction(bPinNumber, GPIO_MODE_INPUT);
#endif
	if (useInternalWeakPullResistors)
	{
#ifndef ESP32_QEMU
		gpio_pulldown_en(aPinNumber);
		gpio_pulldown_en(bPinNumber);
#endif
	}

	// Set up encoder PCNT configuration
	r_enc_config.pulse_gpio_num = aPinNumber; // Rotary Encoder Chan A
	r_enc_config.ctrl_gpio_num = bPinNumber;  // Rotary Encoder Chan B

	r_enc_config.unit = unit;
	r_enc_config.channel = PCNT_CHANNEL_0;

	r_enc_config.pos_mode = fullQuad ? PCNT_COUNT_DEC : PCNT_COUNT_DIS; // Count Only On Rising-Edges
	r_enc_config.neg_mode = PCNT_COUNT_INC;								// Discard Falling-Edge

	r_enc_config.lctrl_mode = PCNT_MODE_KEEP;	 // Rising A on HIGH B = CW Step
	r_enc_config.hctrl_mode = PCNT_MODE_REVERSE; // Rising A on LOW B = CCW Step

	r_enc_config.counter_h_lim = INT16_MAX;
	r_enc_config.counter_l_lim = INT16_MIN;

#ifndef ESP32_QEMU
	pcnt_unit_config(&r_enc_config);
#endif

	if (et == full)
	{
		// set up second channel for full quad
		r_enc_config.pulse_gpio_num = bPinNumber; // make prior control into signal
		r_enc_config.ctrl_gpio_num = aPinNumber;  // and prior signal into control

		r_enc_config.unit = unit;
		r_enc_config.channel = PCNT_CHANNEL_1; // channel 1

		r_enc_config.pos_mode = PCNT_COUNT_DEC; // Count Only On Rising-Edges
		r_enc_config.neg_mode = PCNT_COUNT_INC; // Discard Falling-Edge

		r_enc_config.lctrl_mode = PCNT_MODE_REVERSE; // prior high mode is now low
		r_enc_config.hctrl_mode = PCNT_MODE_KEEP;	 // prior low mode is now high

		r_enc_config.counter_h_lim = INT16_MAX;
		r_enc_config.counter_l_lim = INT16_MIN;

#ifndef ESP32_QEMU
		pcnt_unit_config(&r_enc_config);
#endif
	}
	else
	{											  // make sure channel 1 is not set when not full quad
		r_enc_config.pulse_gpio_num = bPinNumber; // make prior control into signal
		r_enc_config.ctrl_gpio_num = aPinNumber;  // and prior signal into control

		r_enc_config.unit = unit;
		r_enc_config.channel = PCNT_CHANNEL_1; // channel 1

		r_enc_config.pos_mode = PCNT_COUNT_DIS; // disabling channel 1
		r_enc_config.neg_mode = PCNT_COUNT_DIS; // disabling channel 1

		r_enc_config.lctrl_mode = PCNT_MODE_DISABLE; // disabling channel 1
		r_enc_config.hctrl_mode = PCNT_MODE_DISABLE; // disabling channel 1

		r_enc_config.counter_h_lim = INT16_MAX;
		r_enc_config.counter_l_lim = INT16_MIN;

#ifndef ESP32_QEMU
		pcnt_unit_config(&r_enc_config);
#endif
	}

#ifndef ESP32_QEMU
	// Filter out bounces and noise
	pcnt_set_filter_value(unit, 250); // Filter Runt Pulses
	pcnt_filter_enable(unit);

	/* Enable events on  maximum and minimum limit values */
	pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	pcnt_event_enable(unit, PCNT_EVT_L_LIM);

	pcnt_counter_pause(unit); // Initial PCNT init
	pcnt_counter_clear(unit);
#endif
	/* Register ISR handler and enable interrupts for PCNT unit */
	if (ESP32Encoder::attachedInterrupt == false)
	{
		ESP32Encoder::attachedInterrupt = true;
#ifndef ESP32_QEMU
		esp_err_t er = pcnt_isr_register(pcnt_example_intr_handler, (void *)NULL, (int)0,
										 (pcnt_isr_handle_t *)&ESP32Encoder::user_isr_handle);
#else
		esp_err_t er = ESP_OK;
#endif
		if (er != ESP_OK)
		{
			ESP_LOGE("ESP32Encoder", "Encoder wrap interupt failed");
		}
	}
#ifndef ESP32_QEMU
	pcnt_intr_enable(unit);
	pcnt_counter_resume(unit);
#endif
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
	int16_t c;
#ifndef ESP32_QEMU
	pcnt_get_counter_value(unit, &c);
#else
	c = 0;
#endif
	return c;
}
int32_t ESP32Encoder::getCount()
{
	return getCountRaw() + count;
}

int32_t ESP32Encoder::clearCount()
{
	count = 0;
#ifndef ESP32_QEMU
	return pcnt_counter_clear(unit);
#else
	return ESP_OK;
#endif
}

int32_t ESP32Encoder::pauseCount()
{
#ifndef ESP32_QEMU
	return pcnt_counter_pause(unit);
#else
	return ESP_OK;
#endif
}

int32_t ESP32Encoder::resumeCount()
{
#ifndef ESP32_QEMU
	return pcnt_counter_resume(unit);
#else
	return ESP_OK;
#endif
}
