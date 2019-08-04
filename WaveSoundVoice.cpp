#include "WaveSoundVoice.h"


// WaveVoice() constructor
WaveVoice::WaveVoice() {
    
    level = 1;
    sampleRate = getSampleRate();
    
    // ADSR Envelope
    mAttackDuration = 0;
    mAttackCount = 0;
    mAttackRun = false;
    
    mDecayDuration = 0;
    mDecayCount = 0;
    mDecayRun = false;
    
    mSustainLevel = 0;
    mSustainRun = false;
    
    mReleaseDuration = 0;
    mReleaseCount = 0;
    mReleaseRun = false;
    
    mWaveType = 1;   // default set to sine
    
    // wavetable var set up
    tableSize = 128;
    wavetable = new float[ tableSize ];     // allocate appropriate size array
    tableDelta = 2 * MathConstants<double>::pi / tableSize;
    
    currentTableIndex = 0.f;
    tableIndexDelta = 0.f;
}   // END WaveVoice() constructor



//  canPlaySound()
bool WaveVoice::canPlaySound (SynthesiserSound* sound) {
    return dynamic_cast<WaveSound*> (sound) != nullptr;
}   // END canPlaySound



//  startNote()
void WaveVoice::startNote (int midiNoteNumber, float velocity, SynthesiserSound*, int /*currentPitchWheelPosition*/) {
    // reflects how hard a key is hit on certain keyboards.  Level = 1 in constructor.  += avoids level=0 or potential speaker damage
    level += velocity * 0.15;
    
    auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();

    // *** ADSR set up (set to parameter inputs) ***
    mAttackCount = 1;
    mDecayCount = 1;
    mReleaseCount = 1;
    
    mAttackRun = true;      // set to true so attack will work
    mDecayRun = false;
    mSustainRun = false;
    mReleaseRun = false;    // could still be true from previous note
    
    
    /***    wavetable set up    ***/
    currentTableIndex = 0.0;
    tableIndexDelta = cyclesPerSample * tableSize;  // determine number of cycles per table sample
}   // END startNote()



//  stopNote()
void WaveVoice::stopNote (float /*velocity*/, bool allowTailOff) {
    mSustainRun = false;
    mReleaseRun = true;
}   // END stopNote()



//  renderNextBlock()
void WaveVoice::renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) {
    if (tableIndexDelta != 0.0) {
        if ( mReleaseRun != true ) {
            while (--numSamples >= 0) {
                // ADSR Envelope logic
                float outputMult = 0;
                if( mAttackRun ) {
                    outputMult = ( mAttackCount/mAttackDuration );
                    mAttackCount++;
                    
                    if( mAttackCount >= mAttackDuration  ) {
                        mAttackRun = false;
                        mDecayRun = true;
                        mAttackCount = 1;
                    }
                }
                else if( mDecayRun ) {
                    outputMult = 1 - ( (1 - mSustainLevel) * (mDecayCount / mDecayDuration) );
                    //   start at 1        amt to decay     portion to drop for specific sample
                    mDecayCount++;
                    
                    if( mDecayCount >= mDecayDuration ) {
                        mDecayRun = false;
                        mSustainRun = true;
                        mDecayCount = 1;
                    }
                }
                else if( mSustainRun ) {
                    outputMult = mSustainLevel;
                }

                // calculate currentSample and scale with level and ADSR value
                float currentSample = getNextWavetableSample();
                currentSample = currentSample * level * outputMult;
                
                // output to buffer
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample (i, startSample, currentSample);
                
                // post-output actions.  Reset for next sample
                ++startSample;
            }
        }
        else {
            while (--numSamples >= 0) {
                if ( mReleaseRun ) {
                    // release calculations
                    float outputMult = mSustainLevel - ( mSustainLevel * (mReleaseCount / mReleaseDuration) );

                    // calculate currentSample and scale with level and ADSR value
                    float currentSample = getNextWavetableSample();
                    currentSample = currentSample * level * outputMult;
                    
                    // output to buffer
                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);
                    
                    mReleaseCount++;
                    ++startSample;
                    if( mReleaseCount >= mReleaseDuration ) {
                        clearCurrentNote();
                        //angleDelta = 0.0;
                        tableIndexDelta = 0;
                        mReleaseRun = false;
                        break;
                    }   // nested if end
                }   // outer if end
            }   // while end
        }   // mReleaseRun == true else end
    }   // tableIndexDelta if end
}   // END renderNextBlock()



// createWavetable()
void WaveVoice::createWavetable() {
    float currentTableAngle = 0.f;
    
    // function pointer
    float (WaveVoice::* functPtr) (float) = NULL;
    switch( mWaveType ) {       // *** optimize ***     use a function object/pointer to streamline and allow switch to be outside of loop
        case 1:
            functPtr = &WaveVoice::sineWave;
            break;
        case 2:
            functPtr = &WaveVoice::squareWave;
            break;
        case 3:
            functPtr = &WaveVoice::triangleWave;
            break;
        case 4:
            functPtr = &WaveVoice::sawtoothWave;
            break;
    }   // end switch statement
    
    for ( float* ptr = wavetable; ptr != &wavetable[tableSize]; ++ptr ) {
        *ptr = (this->*functPtr) ( currentTableAngle );
        currentTableAngle += tableDelta;        // no need to wrap since tableDelta and currentTableAngle are scaled to 2pi
    }   // end for loop
}   // END createWavetable()



// getNextWavetableSample()
float WaveVoice::getNextWavetableSample() {

    // interpolation set up
    int index0 = currentTableIndex;     // truncate value
    float fraction = currentTableIndex - index0;
    
    int index1 = index0 + 1;
    if (index1 >= tableSize) {      // wrap around tableSize
        index1 -= tableSize;
    }
    
    float value0 = wavetable[ index0 ];
    float value1 = wavetable[ index1 ];
    
    
    // move currentTableIndex forward a sample in the wavetable for next iteration
    currentTableIndex += tableIndexDelta;
    if( currentTableIndex >= tableSize ) {      // wrap around tableSize  ( not around 2pi since table
        currentTableIndex -= tableSize;
    }
    
    // return result of interpolation
    return value0 + fraction * (value1 - value0);
}   // END getNextWavetableSample()
