/*
 * Project ADC
 * Description:
 * Author:
 * Date:
 */
#include <Particle.h>
#include "ADCDMAGen3_RK.h"

SYSTEM_THREAD(ENABLED);
/* PARTICLE CLOUD NOT USED - ONLY WIFI REQUIRED */
SYSTEM_MODE(AUTOMATIC);

/* THREAD SAFE SERIAL OUTPUT */
SerialLogHandler logHandler;

const size_t SAMPLES_IN_BUFFER = 512;
/* NRF SYSTEM DMA CONFIG */
static nrf_saadc_value_t buffer0[SAMPLES_IN_BUFFER];
static nrf_saadc_value_t buffer1[SAMPLES_IN_BUFFER];
static nrf_saadc_value_t *bufferReady = 0;

/* MAX RECORDING TIME */ 
/* MANUALLY START AND STOP RECORDING BY HITTING THE ARGON MODE BUTTON */
const unsigned long MAX_RECORDING_LENGTH_MS = 30000;
unsigned long recordingStart;

/* THIS EXAMPLE EMPLOYS A TCP CONNECTION TO THE NODE APP */
/* THIS IS THE LOCAL SERVER (NODE DESTINATION ADDRESS) ON THE LAN */
TCPClient client;
/* WHERE DATA IS SENT TO */
IPAddress serverAddr = IPAddress(10,0,1,5);
/* PORT FOR COMMUNICATION */
int serverPort = 7123;

/* ADC DMA (DIRECT MEMORY ACCESS) CONTROLLER */
ADCDMAGen3 adc;

/* STATE CONTROL */
enum State { STATE_WAITING, STATE_CONNECT, STATE_RUNNING, STATE_FINISH };
State state = STATE_WAITING;

// Forward declarations
void buttonHandler(system_event_t event, int data);

void setup() {
	// Register handler to handle clicking on the SETUP button
	System.on(button_click, buttonHandler);

	// Blue D7 LED indicates recording is on
	pinMode(D7, OUTPUT);

	// Optional, just for testing so I can see the logs below
	// waitFor(Serial.isConnected, 10000);

	adc.withBufferCallback([](nrf_saadc_value_t *buf, size_t size) {
		// This gets executed after each sample buffer has been read.
		// Note: This is executed in interrupt context, so beware of what you do here!

		// We just remember the buffer and handle it from loop
		bufferReady = buf;
	});

	ret_code_t err = adc
		.withSampleFreqHz(16000)
		.withDoubleBuffer(SAMPLES_IN_BUFFER, buffer0, buffer1)
		.withResolution(NRF_SAADC_RESOLUTION_8BIT)
		.withSamplePin(A0)
		.init();

	Log.info("adc.init %lu", err);
	// Start sampling!
	adc.start();
}

void loop() {

	switch(state) {
	case STATE_WAITING:
		// Waiting for the user to press the MODE button. The MODE button handler
		// will bump the state into STATE_CONNECT
		break;

	case STATE_CONNECT:
		// Ready to connect to the server via TCP
		if (client.connect(serverAddr, serverPort)) {
			// Connected
			Log.info("starting");

			recordingStart = millis();
			digitalWrite(D7, HIGH);

			state = STATE_RUNNING;
		}
		else {
			Log.info("failed to connect to server");
			state = STATE_WAITING;
		}
		break;

	case STATE_RUNNING:
		if (bufferReady) {
			int16_t *src = (int16_t *)bufferReady;
			uint8_t *dst = (uint8_t *)bufferReady;
			bufferReady = 0;

			// The buffer contains 16-bit samples even when sampling at 8 bits!
			// Get rid of the unwanted bytes here
			for(size_t ii = 0; ii < SAMPLES_IN_BUFFER; ii++) {
				dst[ii] = (uint8_t) src[ii];
			}

			// Note: When outputting 16-bit samples you'd write 2 * SAMPLES_IN_BUFFER bytes
			// since each sample is 2 bytes.
			client.write(dst, SAMPLES_IN_BUFFER);
		}

		if (millis() - recordingStart >= MAX_RECORDING_LENGTH_MS) {
			state = STATE_FINISH;
		}
		break;

	case STATE_FINISH:
		digitalWrite(D7, LOW);
		client.stop();
		Log.info("stopping");
		state = STATE_WAITING;
		break;
	}
}

// button handler for the MODE button, used to toggle recording on and off
void buttonHandler(system_event_t event, int data) {
	switch(state) {
	case STATE_WAITING:
		state = STATE_CONNECT;
		break;

	case STATE_RUNNING:
		state = STATE_FINISH;
		break;
	}
}






