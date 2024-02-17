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
    float level; // holds current envelope level
    
    float attackMultiplier;
    float decayMultiplier;
    float sustainLevel;
    float releaseMultiplier;
    
    void reset()
    {
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }
    
    float nextValue()
    {
        // level *= 0.9999f; // 0.9995 is about a half second, 0.9998 is roughly one second
        // level *= multiplier;
        level = multiplier * (level - target) + target;
        if (level + target > 3.0f) {
            // target is 2.0 and level should be more than 1.0
            multiplier = decayMultiplier;
            target = sustainLevel;
        }
        
        return level;
    }
    
    inline bool isActive() const
    {
        return level > SILENCE; // release curve never reaches zero, but will eventually reach SILENCE level
    }
    
    inline bool isInAttack() const
    {
        return target >= 2.0f;
    }
    
    void attack()
    {
        level += SILENCE + SILENCE; // initial envelope level is always greater than SILENCE, isActive() will return true (new notes won't play without this)
        target = 2.0f;
        multiplier = attackMultiplier;
    }
    
    void release()
    {
        target = 0.0f;
        multiplier = releaseMultiplier;
    }

private: // private variables are not accessed from outside the class
    float multiplier;
    float target; // lowpass the "gate" like (binary) signal/trigger for the ADSR, goal of this is to create a smooth exp like function to the target
};
