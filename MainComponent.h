#pragma once

#include "SynthAudioSource.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define MAX_DELAY_TIME 2


class MainComponent   : public AudioAppComponent,
                        private Timer
{
public:
    MainComponent();
    ~MainComponent();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (Graphics& g) override;
    void resized() override;
    
    inline float delay_lin_interp(float sample_x, float sample_x1, float inPhase) { return (1 - inPhase) * sample_x + inPhase * sample_x1; }
    
private:
    void sliderSetUp(Slider* slider, float start_value, float range_low_bound, float range_upper_bound, juce::String name, Label* label, int parameterSynth_index, bool standard, int dec_places );
    void asdrSliderSetUp( Slider* slider, float skewFactor, float skew_midpoint, int parameterSynth_index, float asdr_value, int asdr );
    void timerCallback() override;

    
    SynthAudioSource synthAudioSource;
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboardComponent;
    
    double localSampleRate; // store local sample rate for use in other functions
    float* parametersSynth = NULL; // store and collect paramter float values
    
    // plug-in style sliders
    Slider mDryWetSlider;
    Slider mFeedbackSlider;
    Slider mDelayTimeSlider;
    Slider mGainSlider;
    // ADSR envelope sliders
    Slider mAttackSlider;
    Slider mDecaySlider;
    Slider mSustainSlider;
    Slider mReleaseSlider;
    
    // Combobox
    ComboBox mWaveFormBox;
    ComboBox mDelayTypeBox;
    
    // LFO
    Slider mPhaseOffsetSlider;
    Slider mDepthSlider;
    Slider mRateSlider;
    float mLFOPhase;
    
    // tremelo
    Slider mTremFreqSlider;
    Slider mTremAmountSlider;

    
    // labels
    Label mDryWetLabel;
    Label mFeedbackLabel;
    Label mDelayTimeLabel;
    Label mGainLabel;
    Label mAttackLabel;
    Label mDecayLabel;
    Label mSustainLabel;
    Label mReleaseLabel;
    
    Label mWaveFormLabel;
    Label mDelayTypeLabel;
    
    Label mPhaseOffsetLabel;
    Label mDepthLabel;
    Label mRateLabel;
    
    Label mTremFreqLabel;
    Label mTremAmountLabel;


    // delay and feedback variabless
    float mDryWet;

    float mFeedbackLeft;
    float mFeedbackRight;
    
    float mDelayTimeinSamples;
    float mDelayReadHead;
    float mDelayTimeSmoothed;
    float mDelayTimeDesired;
    
    float* mCircularBufferLeft;
    float* mCircularBufferRight;
    
    int mCircularBufferWriteHead;
    int mCircularBufferLength;
    
    // gain
    float mGainValue;
    float tremeloDelta;
    float tremloCurrentAngle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
