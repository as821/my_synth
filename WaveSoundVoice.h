#ifndef SynthSoundVoice_h
#define SynthSoundVoice_h

#include "../JuceLibraryCode/JuceHeader.h"
#include "Wavetable.h"
#include <math.h>


//  WaveSound struct
struct WaveSound   : public SynthesiserSound
{
    WaveSound() {}
    
    bool appliesToNote    (int) override        { return true; }
    bool appliesToChannel (int) override        { return true; }
};  // END WaveSound struct



// WaveVoice struct
struct WaveVoice   : public SynthesiserVoice
{
    WaveVoice();
    
    bool canPlaySound (SynthesiserSound* sound) override;

    
    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound*, int /*currentPitchWheelPosition*/) override;

    
    void stopNote (float /*velocity*/, bool allowTailOff) override;

    
    void pitchWheelMoved (int) override      {}
    void controllerMoved (int, int) override {}
    
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    
    void setParaterPtr( float* ptr ) { parameterPtr = ptr; }
    
    
    // ADSR setters
    void setAttack( float val ) { mAttackDuration = sampleRate * val; }
    void setDecay( float val )  { mDecayDuration = sampleRate * val; }
    void setSustain( float val ) { mSustainLevel = val; }
    void setRelease( float val ) { mReleaseDuration = sampleRate * val; }
    
    
    // Waveform functions
    void setWaveType( int val ) { mWaveType = val; }
    
    inline float sineWave( float currentAngle ) { return std::sin (currentAngle); }
    inline float triangleWave( float currentAngle ) {
        float mod_calculation = fmod( currentAngle, (2 * MathConstants<double>::pi) );
        return (2 / MathConstants<double>::pi) * abs(mod_calculation - MathConstants<double>::pi) - 1; }
    inline float squareWave( float currentAngle )  { return std::tanh( 7 * std::sin(currentAngle) ); }
    inline float sawtoothWave( float currentAngle) { return fmod(currentAngle,(2*MathConstants<double>::pi)) / MathConstants<double>::pi - 1; }
                                                    // y =               (x % 2pi)                           /           pi - 1
    
    
    // wavetable functions
    void createWavetable();
    float getNextWavetableSample();

private:
    
    // wavetable vars
    float* wavetable;
    int tableSize;
    float tableDelta;
    
    float currentTableIndex;
    float tableIndexDelta;
    
    // additional vars
    float level;
    int sampleRate;
    
    int mWaveType;
    float* parameterPtr;
    
    // ADSR envelope vars
    double mAttackDuration;
    double mAttackCount;
    bool mAttackRun;
    
    double mDecayDuration;
    double mDecayCount;
    bool mDecayRun;
    
    double mSustainLevel;
    bool mSustainRun;
    
    double mReleaseDuration;
    double mReleaseCount;
    bool mReleaseRun;
    
};  // END WaveVoice struct

#endif
