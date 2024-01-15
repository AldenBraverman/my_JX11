/*
  ==============================================================================

    Utils.h
    Created: 14 Jan 2024 7:22:11pm
    Author:  Alden

    // REMOVE FOR RELEASE BUILDS
  ==============================================================================
*/

#pragma once
inline void protectYourEars(float* buffer, int sampleCount)
{
    if (buffer == nullptr) { return; }
    bool firstWarning = true;
    for (int i = 0; i < sampleCount; ++i) {
        float x = buffer[i];
        bool silence = false;

    }
}