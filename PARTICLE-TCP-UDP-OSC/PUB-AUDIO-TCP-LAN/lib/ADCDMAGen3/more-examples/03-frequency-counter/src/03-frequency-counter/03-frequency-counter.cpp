// Shows how to sample using DMA and detect DTMF tones using the Goertzel algorithm
//
// Prints when a key is pressed to the USB serial debug log.
//
// Repository: https://github.com/rickkas7/ADCDMAGen3_RK
// License: MIT (free for use in open or closed source products)

#include "Particle.h"

#include "ADCDMAGen3_RK.h"

#include "Adafruit_SSD1306_RK.h"
#include "FreeSansBold18pt7b.h"

#include "FftComplex.h" // https://www.nayuki.io/page/free-small-fft-in-multiple-languages
#include <cmath> // std::abs

// The Adafruit library defines a macro for abs() which conflicts with std::abs(). Turn that off
#undef abs

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler;

const size_t SAMPLE_FREQ = 200000; // Hz

const size_t SAMPLES_IN_BUFFER = 1024;

static nrf_saadc_value_t buffer0[SAMPLES_IN_BUFFER];
static nrf_saadc_value_t buffer1[SAMPLES_IN_BUFFER];

static nrf_saadc_value_t *bufferReady = 0;

std::vector< std::complex<double> > complexSamples;

ADCDMAGen3 adc;

const unsigned long UPDATE_PERIOD = 100;
unsigned long lastUpdate;

int lastLargestFreq = -1;

// SSD1306 SPI Display Settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_DC     A4
#define OLED_CS     A3
#define OLED_RESET  A5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
		&SPI, OLED_DC, OLED_RESET, OLED_CS);


// Forward declarations


void setup() {
	// Optional, just for testing so I can see the logs below
	// waitFor(Serial.isConnected, 10000);

	// Pre-allocate an array of complex<double> samples in a vector. We reuse this for FFT.
	complexSamples.resize(SAMPLES_IN_BUFFER);

	adc.withBufferCallback([](nrf_saadc_value_t *buf, size_t size) {
		// This gets executed after each sample buffer has been read.
		// Note: This is executed in interrupt context, so beware of what you do here!

		// We just remember the buffer and handle it from loop
		bufferReady = buf;
	});

	// When using high sample frequencies, make sure you reduce AcqTime!

	ret_code_t err = adc
		.withSampleFreqHz(SAMPLE_FREQ)
		.withDoubleBuffer(SAMPLES_IN_BUFFER, buffer0, buffer1)
		.withResolution(NRF_SAADC_RESOLUTION_12BIT)
		.withAcqTime(NRF_SAADC_ACQTIME_3US)
		.withSamplePin(A0)
		.init();

	Log.info("adc.init %lu", err);

	adc.start();

	display.begin(SSD1306_SWITCHCAPVCC);
    display.clearDisplay();
    display.display();

}

void loop() {

	if (bufferReady) {
	}

	if (millis() - lastUpdate >= UPDATE_PERIOD) {
		lastUpdate = millis();

		int16_t *src = (int16_t *)bufferReady;

		// The buffer contains 16-bit samples even when sampling at 8 bits!
		for(size_t ii = 0; ii < SAMPLES_IN_BUFFER; ii++) {
			// We are using 12-bit sampling so values are 0-4095 (inclusive).

			// Create real part from -1.0 to +1.0 instead
			complexSamples[ii] = (double)(src[ii] - 2048) / 2048;
		}

		// Run the FFT on the samples
		Fft::transformRadix2(complexSamples);

		double largestValue = 0.0;
		int largestFreq = -1;

		// Results in the frequency domain are from index 0 to index (SAMPLES_IN_BUFFER / 2)
		// where index 0 is the DC component and the maximum indexex is the highest
		// frequency, which is 1/2 the sampling frequency of 8kHz
		// (The top half of the buffer is the negative frequencies, which we ignore.)
		// Start at 1 to ignore the DC component
		for(size_t ii = 1; ii < SAMPLES_IN_BUFFER / 2; ii++) {
			int freq = ii * SAMPLE_FREQ / SAMPLES_IN_BUFFER;
			double value = std::abs(complexSamples[ii].real());
			if (value > 100.0 && value > largestValue) {
				largestValue = value;
				largestFreq = freq;
			}
		}
	    display.clearDisplay();

		if (largestFreq > 0) {
			Log.info("freq=%d", largestFreq);
			lastLargestFreq = largestFreq;
		}

		if (lastLargestFreq > 0) {
		    display.setTextColor(WHITE);
		    display.setTextSize(1);
		    display.setFont(&FreeSansBold18pt7b);
		    display.setCursor(0, 40);
		    display.println(lastLargestFreq);
		}

	    display.display();
	}


}


