/*
  ==============================================================================

    Envelope.h
    Created: 6 Feb 2024 8:08:28pm
    Author:  Alden

  ==============================================================================
*/

#pragma once
const float SILENCE = 0.0001f; // calculating the multiplier

class Envelope
{
public:
    float multiplier;
    
    float target; // lowpass the "gate" like (binary) signal/trigger for the ADSR, goal of this is to create a smooth exp like function to the target
    
    float nextValue()
    {
        // level *= 0.9999f; // 0.9995 is about a half second, 0.9998 is roughly one second
        // level *= multiplier;
        level = multiplier * (level - target) + target;
        return level;
    }

    float level; // holds current envelope level
};
