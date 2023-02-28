#pragma once
namespace KernelBeep
{
	BOOL Beep(DWORD dwFreq, DWORD dwDuration);
};

#define TIMER_FREQUENCY         1193167//; 1, 193, 167 Hz
#define OCTAVE                  2//; octave multiplier

#define PITCH_C                 523//; C - 523, 25 Hz
#define PITCH_G                 784//; G - 783, 99 Hz


#define TONE_1                  TIMER_FREQUENCY / (PITCH_C * OCTAVE)
#define TONE_2                 (PITCH_C * OCTAVE)//; for HalMakeBeep
#define TONE_3                 (PITCH_G * OCTAVE)//; for HalMakeBeep

