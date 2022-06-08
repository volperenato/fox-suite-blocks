#pragma once
class Delay
{
protected:

	// sample rate value from the Host
	int dly_sampleRate;

	// Delay line maximum Length in samples
	int dly_lineLengthInSamples;

	// Delay line maximum Length in milliseconds
	int dly_lineLengthInmsec;

	// Delay line Length in samples
	float dly_delayInSamples;

	// Delay line Length in milliseconds
	float dly_delayInmsec;

	// Reading/writing indices
	int dly_readIndex, dly_writeIndex;

	// Delay Line Buffer
	float* dly_buffer;

	// output attenuation in dB
	float dly_makeUpGaindB;

	// output attenuation in linear value (0->1)
	float dly_makeUpGain;

public:

	Delay();
	~Delay();
	virtual void init(float delayMaximumLengthInmsec, int sampleRate);
	void initInSamples(int delayLengthInSamples, int sampleRate);
	void initDelayLine();
	void updateParameters();
	void reset();
	virtual void setSampleRate(int sampleRate);
	void setDelayInmsec(float delayInmsec);
	void setMakeUpGaindB(float gaindB);
	void setMakeUpGainLin(float gainLin);
	void updateIndices();
	void writeToDelayLine(float xn);
	float readFromDelayLine();
	float linearInterp(float x1, float x2, float y1, float y2, float x);
	virtual float processAudio(float xn);
};