#pragma once
#include "Delay.h"
#include <string>
#include "utils.h"

Delay::Delay()
{
	dly_buffer				= nullptr;
	dly_makeUpGaindB		= 0.0;
	dly_makeUpGain			= 1.0;
	dly_delayInmsec			= 0.0;
	dly_delayInSamples		= 0.0;
	dly_readIndex			= 0;
	dly_writeIndex			= 0;
	dly_lineLengthInSamples = 0;
	dly_lineLengthInmsec	= 0.0;
}


Delay::~Delay()
{
	freeBuffer();
}
 
void Delay::init(float maxDelayInmsec, int sampleRate)
{
	// set delay line length in milliseconds
	dly_lineLengthInmsec = maxDelayInmsec;

	// allocate sample rate
	dly_sampleRate = sampleRate;

	// set delay line length in samples
	dly_lineLengthInSamples = dly_lineLengthInmsec * dly_sampleRate / 1000;

	// set delay msec
	dly_delayInmsec = dly_lineLengthInmsec;

	// set delay in samples
	dly_delayInSamples = dly_lineLengthInSamples;

	// init read and write indices
	dly_readIndex = 0;
	dly_writeIndex = 0;

	// initialize delay line
	initDelayLine();
}

void Delay::initInSamples(int delayLengthInSamples, int sampleRate)
{
	// set delay line length in samples
	dly_lineLengthInSamples = delayLengthInSamples;

	// set internal sample rate value
	dly_sampleRate = sampleRate;

	// set delay line length in msec
	dly_lineLengthInmsec = dly_lineLengthInSamples * 1000 / dly_sampleRate;

	// set delay msec
	dly_delayInmsec = dly_lineLengthInmsec;

	// set delay in samples
	dly_delayInSamples = dly_lineLengthInSamples;

	// init delay line buffer
	initDelayLine();
}

void Delay::initDelayLine()
{
	// free the delay buffer in case it had already been allocated
	freeBuffer(); 

	// define delay line length in bytes
	int lineLengthInBytes = dly_lineLengthInSamples * sizeof(float);

	// allocate memory for the delay line
	dly_buffer = (float*)malloc(lineLengthInBytes);

	// set the allocated memory at zero
	memset(dly_buffer, 0, lineLengthInBytes);
}


void Delay::updateParameters() 
{
	// convert makeup gain in linear value
	dly_makeUpGain = pow(10.0, dly_makeUpGaindB / 20.0);

	// define delay size in samples
	dly_delayInSamples = dly_delayInmsec * (float)dly_sampleRate / 1000;

	// protection against a 0 samples delay
	if (dly_delayInSamples == 0)
		dly_delayInSamples = 1;

	// compute read index as the write index minus the delay length
	dly_readIndex = dly_writeIndex - dly_delayInSamples;

	// check if the read index is negative. In that case wrap it by adding the delay line length
	if (dly_readIndex < 0)
		dly_readIndex += dly_lineLengthInSamples;
}

void Delay::freeBuffer() {
	if (dly_buffer)
		delete dly_buffer;
	dly_buffer = nullptr;
}

void Delay::setSampleRate(int sampleRate)
{
	// temporarily store delay length in milliseconds
	float dlyInMS = dly_delayInmsec;
	
	// initialize the delay from scratches using the same maximum delay length value but new sample rate
	init(dly_lineLengthInmsec, sampleRate);

	// set the old delay length in milliseconds
	setDelayInmsec(dlyInMS);
}

void Delay::setDelayInmsec(float delayInmsec)
{
	// Set delay line length in milliseconds
	dly_delayInmsec = delayInmsec;

	if (dly_delayInmsec > dly_lineLengthInmsec)
		dly_delayInmsec = dly_lineLengthInmsec;

	// Update parameters based on new delay length
	updateParameters();
}

void Delay::setMakeUpGaindB(float gaindB)
{
	// set make up gain [dB]
	dly_makeUpGaindB = gaindB;

	// update parameters
	updateParameters();
}

void Delay::setMakeUpGainLin(float gainLin)
{
	// set make up gain [lin]
	dly_makeUpGain = gainLin;

	// compute makeup gain in dB
	dly_makeUpGaindB = 20 * log10(dly_makeUpGain);

}

void Delay::updateIndices()
{
	// Increase reading index
	dly_readIndex++;

	// check if reading index is out of delay line length
	if (dly_readIndex >= dly_lineLengthInSamples)
		dly_readIndex = 0;

	// Increase writing index
	dly_writeIndex++;

	// check if writing index is out of delay line length
	if (dly_writeIndex >= dly_lineLengthInSamples)
		dly_writeIndex = 0;
}

void Delay::writeToDelayLine(float xn)
{
	// write the sample 'x' to current writing position of delay buffer
	dly_buffer[dly_writeIndex] = xn;
}

float Delay::readFromDelayLine()
{
	// read the current sample from delay line buffer
	float yn = dly_buffer[dly_readIndex];

	// compute previous index and wrap if needed
	int readIndex_1 = dly_readIndex - 1;
	if (readIndex_1 < 0)
		readIndex_1 = dly_lineLengthInSamples - 1;

	// read previous sample from delay line
	float yn_1 = dly_buffer[readIndex_1];

	// compute the fractional part of the delay in samples
	float frac = dly_delayInSamples - (int)dly_delayInSamples;

	// compute the interpolated delay value
	float interp = linearInterp(0, 1, yn, yn_1, frac);
	
	return interp;
}

float Delay::processAudio(float xn)
{
	// read delay sample
	float yn = readFromDelayLine();

	// allocate value to delay line
	writeToDelayLine(xn);

	// Update read/write indices
	updateIndices();

	return yn * dly_makeUpGain;
}
