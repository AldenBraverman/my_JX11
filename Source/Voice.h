/*
  ==============================================================================

    Voice.h
    Created: 12 Jan 2024 7:38:30pm
    Author:  Alden

  ==============================================================================
*/

#pragma once
struct Voice // produce the next output sample for a given note
{
    int note;
    int velocity;

    void reset() // also for initialization
    {
        note = 0;
        velocity = 0;
    }
};