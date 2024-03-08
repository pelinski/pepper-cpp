/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io
*/
/**
\example Pepper/getting-started-cpp/render.cpp

Author: Teresa Pelinski (@pelinski)
Debouncer class taken from the C++ Real-Time Audio Programming with Bela course
(Lecture 14: ADSR)

This example contains everything you need to get started with PEPPER and is a
good starting point for new projects. It is a cpp version of the
Pepper/getting-started example Pd patch. We initialise the buttons and LEDs,
print when a button is pushed, flash leds in a sequence, pass CV in to CV out,
pass audio in to audio out, and generate a test tone. We also send the first
three channels of the CV inputs to the scope. Button debouncing is used to avoid
multiple readings of the same button press.

*/

#include "Debouncer.h"
#include <Bela.h>
#include <cmath>
#include <libraries/Scope/Scope.h>

Scope scope;

int gAudioFramesPerAnalogFrame = 0;
float gInverseSampleRate;

// two oscillators
float gFrequency[2] = {330.0, 0.25};
float gPhase[2] = {0.0, 0.0};

// array to save the first 3 channels of CV inputs to log in the scope
float gScopeData[3] = {0.0, 0.0, 0.0};

// Buttons
unsigned int gButtonPins[4] = {15, 14, 13, 12}; // left to right
unsigned int gPrevButtonValue[4] = {0, 0, 0, 0};
Debouncer gDebouncer[4];

// LEDs
float gLedPins[10] = {6, 7, 10, 2, 3, 0, 1, 4, 5, 8}; // left to right
unsigned int gMetroInterval = 0;
unsigned int gMetroCount = 0;
unsigned int gLedIdx = 0;

bool setup(BelaContext *context, void *userData) {

  if (context->analogSampleRate > context->audioSampleRate) {
    fprintf(stderr, "Error: for this project the sampling rate of the analog "
                    "inputs has to be <= the audio sample rate\n");
    return false;
  }
  if (context->analogInChannels < 2) {
    fprintf(stderr,
            "Error: for this project you need at least two analog inputs\n");
    return false;
  }
  if (context->analogFrames)
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
  gInverseSampleRate = 1.0 / context->audioSampleRate;

  // set up digital IO pins
  for (unsigned int i = 0; i < 10; i++) {
    pinMode(context, 0, gLedPins[i], OUTPUT);
  }
  for (unsigned int i = 0; i < 4; i++) {
    pinMode(context, 0, gButtonPins[i], INPUT);
  }

  // set up Button debouncers (one per button)
  for (unsigned int i = 0; i < 4; i++) {
    gDebouncer[i].setup(context->audioSampleRate, .05);
  }

  // interval at which the LEDs will be toggled
  gMetroInterval = context->digitalSampleRate; // digital frames in 1s

  // setup the scope with 3 channels at the audio sample rate
  scope.setup(3, context->audioSampleRate);

  return true;
}

void render(BelaContext *context, void *userData) {

  for (unsigned int n = 0; n < context->audioFrames; n++) {

    // two oscillators to add to the input audio
    float osc = 0.0;
    for (unsigned int i = 0; i < 2; i++) {
      osc += 0.8f * sinf(gPhase[i]);
      gPhase[i] += 2.0f * (float)M_PI * gFrequency[i] * gInverseSampleRate;
      if (gPhase[i] > M_PI)
        gPhase[i] -= 2.0f * (float)M_PI;
    }

    // audio in plus two oscillators to out
    for (unsigned int ch = 0; ch < context->audioInChannels; ch++) {
      float out = audioRead(context, n, ch) + osc;
      audioWrite(context, n, ch, out);
    }

    if (gAudioFramesPerAnalogFrame && !(n % gAudioFramesPerAnalogFrame)) {
      // pass CV in to CV out
      for (unsigned int channel = 0; channel < context->analogInChannels;
           channel++) {
        float _in =
            analogRead(context, n / gAudioFramesPerAnalogFrame, channel);
        analogWrite(context, n, channel, _in);
        // add first 3 CV in channels to scopeData
        if (channel < 3) {
          gScopeData[channel] = _in;
        }
      }
    }

    for (unsigned int n = 0; n < context->digitalFrames; n++) {

      // print button values on change
      for (unsigned int i = 0; i < 4; i++) {
        unsigned int buttonValue =
            gDebouncer[i].process(digitalRead(context, n, gButtonPins[i]));
        if (buttonValue != gPrevButtonValue[i]) {
          rt_printf("Button %d: %d\n", i, buttonValue);
          gPrevButtonValue[i] = buttonValue;
        }
      }

      // toggle LEDs every gMetroInterval
      if (++gMetroCount >= gMetroInterval) {
        gMetroCount = 0;
        digitalWrite(context, n, gLedPins[gLedIdx], 1);
        if (gLedIdx > 0) {
          digitalWrite(context, n, gLedPins[gLedIdx - 1], 0);
        } else {
          digitalWrite(context, n, gLedPins[9], 0);
        }
        if (++gLedIdx > 9) {
          gLedIdx = 0;
        }
      }
    }

    // log first 3 channels of CV inputs
    scope.log(gScopeData[0], gScopeData[1], gScopeData[2]);
  }
}

void cleanup(BelaContext *context, void *userData) {}
