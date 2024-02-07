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
    float nextValue()
    {
        level *= 0.9999f; // 0.9995 is about a half second, 0.9998 is roughly one second
        return level;
    }

    float level; // holds current envelope level
};