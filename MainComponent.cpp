#include "MainComponent.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define NUM_PARAMETERS 14


        /*** public member functions ***/

//  MainComponent() constructor
MainComponent::MainComponent() : synthAudioSource  (  keyboardState ),
keyboardComponent (  keyboardState, MidiKeyboardComponent::horizontalKeyboard ) {
    
    AudioAppComponent::setSize (800, 600);
    AudioAppComponent::addAndMakeVisible (keyboardComponent);
    
    // audio processing vars
    parametersSynth = new float[ NUM_PARAMETERS ];       // number of relevant parameters.  Stores value.  Updated by .onValueChange lambda
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    mDelayTimeinSamples = 0;
    mDelayReadHead = 0;
    mFeedbackLeft = 0;
    mFeedbackRight = 0;
    mDryWet = 0.5;
    mDelayTimeSmoothed = 0;
    mDelayTimeDesired = 0;
    mGainValue = 0.8;
    
    // LFO
    mLFOPhase = 0.f;
    
    // tremelo
    tremeloDelta = 0.f;
    tremloCurrentAngle = 0.f;
    
    /***    slider set up   ***/
    //mDryWetSlider set up
    sliderSetUp(&mDryWetSlider, 0.5f, 0.f, 1.f, "Dry/Wet", &mDryWetLabel, 0, true, 2);
    
    // mFeedbackSlider set up
    sliderSetUp(&mFeedbackSlider, 0.1f, 0.0f, 0.98f, "Feedback", &mFeedbackLabel, 1, true, 2);
    
    // mDelayTimeSlider set up
    sliderSetUp(&mDelayTimeSlider, 0.1f, 0.0f, MAX_DELAY_TIME, "Delay Time", &mDelayTimeLabel, 2, true, 2);
    mDelayTimeDesired = 0.1f;
    
    // mGainSlider set up
    sliderSetUp(&mGainSlider, 1.f, 0.f, 2.f, "Gain", &mGainLabel, 3, true, 2);
    
    // mPhaseOffsetSlider set up
    sliderSetUp(&mPhaseOffsetSlider, 0.f, 0.f, 1.f, "Phase Offset", &mPhaseOffsetLabel, 9, true, 2);
    
    // mRateSlider set up
    sliderSetUp(&mRateSlider, 10.f, 0.1f, 20.f, "Rate", &mRateLabel, 10, true, 1);
    
    // mDepthSlider set up
    sliderSetUp(&mDepthSlider, 0.5f, 0.f, 1.f, "Depth", &mDepthLabel, 11, true, 2);
    
    // mTremFrewSlider set uo
    sliderSetUp(&mTremFreqSlider, 10, 1, 30, "Trem. Freq.", &mTremFreqLabel, 12, true, 0);
    mTremFreqSlider.setSkewFactorFromMidPoint( 10 );
    
    // mTremAmountSlider set up
    sliderSetUp(&mTremAmountSlider, 0.f, 0.f, 1.f, "Trem. Amt.", &mTremAmountLabel, 13, true, 2);
    mTremAmountSlider.setSkewFactorFromMidPoint( 0.3f );

    
    
    // ASDR sliders
    // mAttackSlider set up
    sliderSetUp(&mAttackSlider, 0.05f, 0.001f, 4.f, "Attack", &mAttackLabel, 4, false, 3);
    mAttackSlider.setSkewFactor(0.5f);              // skew ( <1 to have better magnification on low end)
    mAttackSlider.setSkewFactorFromMidPoint(1.0f);
    float* ptr4 = &parametersSynth[4];
    mAttackSlider.onValueChange = [this, ptr4] {
        *ptr4 = mAttackSlider.getValue();
        for( int i = 0; i < NUM_VOICES; i++ ) {
            WaveVoice* voice = (WaveVoice *) synthAudioSource.synth.getVoice(i);  // WaveVoice is derived class of SynthesizerVoice, allow cast
            voice->setAttack( *ptr4 );  }
    };
    
    // mDecaySlider set up
    sliderSetUp(&mDecaySlider, 0.05f, 0.001f, 4.f, "Decay", &mDecayLabel, 5, false, 3);
    float* ptr5 = &parametersSynth[5];
    mDecaySlider.setSkewFactor(0.5f);              // skew ( <1 to have better magnification on low end)
    mDecaySlider.setSkewFactorFromMidPoint(1.0f);
    mDecaySlider.onValueChange = [this, ptr5] {
        *ptr5 = mDecaySlider.getValue();
        for( int i = 0; i < NUM_VOICES; i++ ) {
            WaveVoice* voice = (WaveVoice *) synthAudioSource.synth.getVoice(i);  // WaveVoice is derived class of SynthesizerVoice, allow cast
            voice->setDecay( *ptr5 );   }
    };
    
    // mSustainSlider set up
    sliderSetUp(&mSustainSlider, 0.4f, 0.001f, 1.f, "Sustain", &mSustainLabel, 6, false, 3);
    float* ptr6 = &parametersSynth[6];
    mSustainSlider.setSkewFactor(1.5f);              // skew ( >1 to have better magnification on upper end)
    mSustainSlider.onValueChange = [this, ptr6] {
        *ptr6 = mSustainSlider.getValue();
        for( int i = 0; i < NUM_VOICES; i++ ) {
            WaveVoice* voice = (WaveVoice *) synthAudioSource.synth.getVoice(i);  // WaveVoice is derived class of SynthesizerVoice, allow cast
            voice->setSustain( *ptr6 );     }
    };
    
    // mReleaseSlider set up
    sliderSetUp(&mReleaseSlider, 0.3f, 0.001f, 4.f, "Release", &mReleaseLabel, 7, false, 3);
    float* ptr7 = &parametersSynth[7];
    mReleaseSlider.setSkewFactor(0.5f);              // skew ( <1 to have better magnification on low end)
    mReleaseSlider.setSkewFactorFromMidPoint(1.5f);
    mReleaseSlider.onValueChange = [this, ptr7] {
        *ptr7 = mReleaseSlider.getValue();
        for( int i = 0; i < NUM_VOICES; i++ ) {
            WaveVoice* voice = (WaveVoice *) synthAudioSource.synth.getVoice(i);  // WaveVoice is derived class of SynthesizerVoice, allow cast
            voice->setRelease( *ptr7 );     }
        
    };

    
    
    /***    non-slider component set up    ***/
    // Waveform ComboBox
    mWaveFormBox.addItem("Sine", 1);
    mWaveFormBox.addItem("Square", 2);
    mWaveFormBox.addItem("Triangle", 3);
    mWaveFormBox.addItem("Sawtooth", 4);
    mWaveFormBox.setSelectedId(1);
    AudioAppComponent::addAndMakeVisible(mWaveFormBox);
    mWaveFormBox.onChange = [this] {
        for( int i = 0; i < NUM_VOICES; i++ ) {
            WaveVoice* voice = (WaveVoice *) synthAudioSource.synth.getVoice(i);  // WaveVoice is derived class of SynthesizerVoice, allow cast
            voice->setWaveType( mWaveFormBox.getSelectedId() );
            voice->createWavetable();
        }
        keyboardComponent.toFront( true );      // resets focus back to keyboard so MIDI keys can be played
    };
    mWaveFormLabel.setText("Waveform Selector", dontSendNotification);
    mWaveFormLabel.attachToComponent(&mWaveFormBox, false);
    mWaveFormLabel.setJustificationType (Justification::centred);
    
    
    // delay combobox
    // mDelayTypeBox Combobox
    mDelayTypeBox.addItem("Delay", 1);
    mDelayTypeBox.addItem("Chorus", 2);
    mDelayTypeBox.addItem("Flanger", 3);
    mDelayTypeBox.setSelectedId(1);
    AudioAppComponent::addAndMakeVisible(mDelayTypeBox);
    parametersSynth[8] = 1;         // delay is default selection
    float* ptr8 = &parametersSynth[8];
    mDelayTypeBox.onChange = [this, ptr8] {
        *ptr8 = mDelayTypeBox.getSelectedId();
        keyboardComponent.toFront( true );      // resets focus back to keyboard so MIDI keys can be played
    };
    mDelayTypeLabel.setText("Delay Selector", dontSendNotification);
    mDelayTypeLabel.attachToComponent(&mDelayTypeBox, false);
    mDelayTypeLabel.setJustificationType (Justification::centred);
    
    
    
    
    
    
    
    
    /***    runtime permissions     ***/
    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio) && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio)){
        RuntimePermissions::request (RuntimePermissions::recordAudio, [&] (bool granted) {
            if (granted)
                setAudioChannels (0, 2);
        } );
    }
    else {
        setAudioChannels (0, 2);        // Specify the number of input and output channels we want to open
    }
    
    
    /***    pass parameterValue array to all WaveVoices ***/
    for( int i = 0; i < NUM_VOICES; i++ ) {
        WaveVoice* voice = (WaveVoice *) synthAudioSource.synth.getVoice(i);  // WaveVoice is a derived class of SynthesizerVoice, allow cast
        voice->setParaterPtr( parametersSynth );
    }
    
    
    // set bounds dynamically in relation to other components
    resized();
    
    // start MIDI timestamp timer.  400 is arbitrary and probably too low for this application, but it works
    startTimer (400);
}   // END MainComponent() constructor



//  MainComponent destructor
MainComponent::~MainComponent() {
    // clear CircularBuffers
    if (mCircularBufferLeft != nullptr) {
        delete[] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }
    
    if (mCircularBufferRight != nullptr) {
        delete[] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }
    
    // clear parameter array
    delete[] parametersSynth;
    
    shutdownAudio();
}   // END MainComponent destructor



//  prepareToPlay()
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    // *** synth prepareToPlay() ***
    synthAudioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    
    // *** delay/feedback prepareToPlay() ***
    localSampleRate = sampleRate;
    mDelayTimeDesired = parametersSynth[2];    // delayTime param value
    mDelayTimeinSamples = localSampleRate * mDelayTimeSmoothed;   // delayTime param value
    
    // delay storage setup
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    mCircularBufferWriteHead = 0;
    
    // set CircularBufferSizes
    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float [mCircularBufferLength];
        zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));    // zero buffer contents
    }
    
    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float [mCircularBufferLength];
        zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));   // zero buffer contents
    }
}   // END prepareToPlay(ä



//  getNextAudioBlock()
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) {
    
    bufferToFill.clearActiveBufferRegion();     // can take this out if wanted
    synthAudioSource.getNextAudioBlock (bufferToFill);      // MIDI values inputted
    
    // *** delay section ***       bufferToFill.buffer is the AudioBuffer<float> to be edited
    float* leftChannel = bufferToFill.buffer->getWritePointer(0);
    float* rightChannel = bufferToFill.buffer->getWritePointer(1);

    // processing loop
    for( int i = 0; i < bufferToFill.buffer->getNumSamples(); i++ ) {
        
        // gain var and tremelo
        float gain = parametersSynth[3];
        float trem_freq = parametersSynth[ 12 ];
        float trem_amount = parametersSynth[ 13 ];
        
        tremeloDelta = 2*MathConstants<double>::pi * ( trem_freq / localSampleRate );
        float tremelo = sin( tremloCurrentAngle + 1 ) * trem_amount;
        tremloCurrentAngle += tremeloDelta;
        gain += tremelo;
        
        
        
        
        // delay controls
        int delay_type = parametersSynth[8];
        
        float delay_sample_left = 0.f;
        float delay_sample_right = 0.f;
        
        
        if( delay_type == 1 ) {
            mDelayTimeSmoothed = mDelayTimeSmoothed + 0.01 * (mDelayTimeDesired - mDelayTimeSmoothed);  // smooth delay time
            mDelayTimeinSamples = localSampleRate * mDelayTimeSmoothed;   // this must occur after the line above it so it gets smoothed first
            
            // *** output to delay buffer ***
            mCircularBufferLeft[mCircularBufferWriteHead] = (leftChannel[i] + mFeedbackLeft);
            mCircularBufferRight[mCircularBufferWriteHead] = (rightChannel[i] + mFeedbackRight);
            
            // find delay read head
            mDelayReadHead = mCircularBufferWriteHead - mDelayTimeinSamples;
            if(mDelayReadHead < 0) {
                mDelayReadHead += mCircularBufferLength;
            }
            
            //interpolation set up
            int readHead_x = (int)mDelayReadHead;
            int readHead_x1 = readHead_x + 1;
            if (readHead_x1 >= mCircularBufferLength) {
                readHead_x -= mCircularBufferLength;
            }
            float readHeadFloat = mDelayReadHead - readHead_x;
            
            // interpolation
            delay_sample_left = delay_lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
            delay_sample_right = delay_lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);
            
        }       // end delay if
        else {      //  chorus or flanger  (styles of pitch modulation)
            
            float phase_offset_parameter = parametersSynth[ 9 ];
            float rate_parameter = parametersSynth[ 10 ];
            float depth_parameter = parametersSynth[ 11 ];

           // * generate left lfo output *
            float lfoOutLeft = sin(2*M_PI * mLFOPhase);     // a selected waveform
            
            // calculate right lfo phase (must be less than 1) *
            float lfoPhaseRight = mLFOPhase + phase_offset_parameter;
            if( lfoPhaseRight > 1 ) { lfoPhaseRight -= 1; }
            
            // generate right lfo output * /
            float lfoOutRight = sin(2*M_PI * lfoPhaseRight);
            
            // * moving LFO phase forward * /
            //mLFOPhase += (rate_parameter / localSampleRate);     // set phase for LFO for next block
            mLFOPhase += rate_parameter / localSampleRate;
            if( mLFOPhase > 1 ) {   // ensure LFO phase is between desired 0 and 1 at all times
                mLFOPhase -= 1;
            }
            
            
           // * control lfo depth *
            lfoOutLeft *= depth_parameter;  // set depth for LFOs
            lfoOutRight *= depth_parameter;
            
            
            // map lfo outputs to desired delay times * /
            float lfoOutMappedLeft = 0;
            float lfoOutMappedRight = 0;
            
            // chorus effect
            if ( delay_type == 2 ) {
                lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);     // for chorus effect, between 5 and 30 ms
                lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
            }
            else if ( delay_type == 3) {      // flanger effect
                lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.05f);     // for flange effect, between 1 and 5 ms
                lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.05f);
            }
            
            //  * calculate delay length in samples * /
            float delayTimeSampleLeft = localSampleRate * lfoOutMappedLeft;  // lfoOut is a smooth sine wave already
            float delayTimeSampleRight = localSampleRate * lfoOutMappedRight;
            
            
            // * calculate L/R read head positions * /
            float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSampleLeft;
            if( delayReadHeadLeft < 0) { delayReadHeadLeft += mCircularBufferLength; }
            
            float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSampleRight;
            if( delayReadHeadRight < 0) { delayReadHeadRight += mCircularBufferLength; }
            
            
            // * calculate linear interpolation points for left channel * /
            int readHeadLeft_x = (int)delayReadHeadLeft;
            int readHeadLeft_x1 = readHeadLeft_x + 1;
            float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;
            if (readHeadLeft_x1 >= mCircularBufferLength) {
                readHeadLeft_x -= mCircularBufferLength;
            }
            
            
            // * calculate linear interpolation points for left channel * /
            int readHeadRight_x = (int)delayReadHeadRight;
            int readHeadRight_x1 = readHeadRight_x + 1;
            float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;
            if (readHeadRight_x1 >= mCircularBufferLength) {
                readHeadRight_x -= mCircularBufferLength;
            }
            
            
            // * generate left and right output samples * /
            delay_sample_left = delay_lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
            delay_sample_right = delay_lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);
        }      // end delay type if/else structure
        
        
        
        
        // same for all dely types
        // feedback calculation
        mFeedbackLeft = delay_sample_left * parametersSynth[1];
        mFeedbackRight = delay_sample_right * parametersSynth[1];
        
        // calculate left/right out
        float drywet_parameter = parametersSynth[ 0 ];
        float left_out = gain * ( bufferToFill.buffer->getSample(0, i) * (1 - drywet_parameter) + mFeedbackLeft * drywet_parameter );
        float right_out = gain * ( bufferToFill.buffer->getSample(1, i) * (1 - drywet_parameter) + mFeedbackRight * drywet_parameter );
        
        
        // final insertion to buffer (output here)
        // feedback and delay both included here
        bufferToFill.buffer->setSample(0, i, left_out );
        bufferToFill.buffer->setSample(1, i, right_out );
        
        
        // post-output actions
        mCircularBufferWriteHead++;
        
        // reset circular buffer write head
        if(mCircularBufferWriteHead >= mCircularBufferLength) {
            mCircularBufferWriteHead = 0;
        }
    }       // end of buffer sample loop
    
    
    
    
    
    
    
    
    if (parametersSynth[0] == 0) {      // if dry/wet param is 0, then clear the buffers
        // delay storage setup
        mCircularBufferWriteHead = 0;
        
        // clear delay circular buffers
        zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));
        zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));
    }
    
}   // END gwtNextAudioBlock()



//  releaseResources()
void MainComponent::releaseResources() {
    synthAudioSource.releaseResources();
}   // END releaseResources()



//  paint()
void MainComponent::paint (Graphics& g) {
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}   // END paint()



//  resized()
void MainComponent::resized() {
    
    keyboardComponent.setBounds ( 25, getHeight() - 125, getWidth() - 50, 100);
                              //  x        y               wid            hei
    
    /***    slider resizing     ***/
    int slider_height = 95;
    int slider_width = 95;
    int label_height = 25;
    int label_width = 75;
    
    int combo_label_height = 30;
    int combo_label_width = 100;
    //int combo_label_x_dif = 5;
    int combo_label_y_dif = 25;
    
    int left_margin = 10;
    int top_margin = 20;
    int space_between_rows = 115;
    int space_between_columns = 100;
    
    int label_x_dif = 10;
    int label_y_dif = 17;
    
    //  gain
    mGainSlider.setBounds( left_margin, top_margin, slider_width, slider_height );
    mGainLabel.setBounds( mGainSlider.getX() + label_x_dif, mGainSlider.getY() - label_y_dif, label_width, label_height );
    
    // wavefrom combobox
    mWaveFormBox.setBounds( mGainSlider.getX() + space_between_columns + 20, mGainSlider.getY() + 10, 100, 25 );
    mWaveFormLabel.setBounds( mWaveFormBox.getX() /*+ combo_label_x_dif*/ , mWaveFormBox.getY() - combo_label_y_dif, combo_label_width, combo_label_height );
    mWaveFormLabel.setFont( Font( 15.f, Font::plain ) );
    
    //  delay type combobox
    mDelayTypeBox.setBounds( mWaveFormBox.getX() + space_between_columns + 20, mWaveFormBox.getY(), 100, 25);
    mDelayTypeLabel.setBounds( mDelayTypeBox.getX(), mDelayTypeBox.getY() - combo_label_y_dif, combo_label_width, combo_label_height);
    mDelayTypeLabel.setFont( Font( 13.f, Font::plain ) );
    
    //  attack
    mAttackSlider.setBounds( left_margin, mGainSlider.getY() + space_between_rows, slider_width, slider_height );
    mAttackLabel.setBounds( mAttackSlider.getX() + label_x_dif, mAttackSlider.getY() - label_y_dif , label_width, label_height );
    
    // decay
    mDecaySlider.setBounds( mAttackSlider.getX() + space_between_columns, mAttackSlider.getY(), slider_width, slider_height );
    mDecayLabel.setBounds( mDecaySlider.getX() + label_x_dif, mDecaySlider.getY() - label_y_dif, label_width, label_height );
    
    // sustain
    mSustainSlider.setBounds( mDecaySlider.getX() + space_between_columns, mAttackSlider.getY(), slider_width, slider_height);
    mSustainLabel.setBounds( mSustainSlider.getX() + label_x_dif, mSustainSlider.getY() - label_y_dif, label_width, label_height );
    
    //  release
    mReleaseSlider.setBounds( mSustainSlider.getX() + space_between_columns, mAttackSlider.getY(), slider_width, slider_height );
    mReleaseLabel.setBounds( mReleaseSlider.getX() + label_x_dif, mReleaseSlider.getY() - label_y_dif , label_width , label_height );
    
    //  dry/wet
    mDryWetSlider.setBounds( left_margin, mAttackSlider.getY() + space_between_rows, slider_width, slider_height );
    mDryWetLabel.setBounds( mDryWetSlider.getX() + label_x_dif, mDryWetSlider.getY() - label_y_dif, label_width, label_height );
    
    //  feedback
    mFeedbackSlider.setBounds( mDryWetSlider.getX() + space_between_columns, mDryWetSlider.getY() , slider_width , slider_height );
    mFeedbackLabel.setBounds( mFeedbackSlider.getX() + label_x_dif, mFeedbackSlider.getY() - label_y_dif, label_width , label_height );

    //  delay time
    mDelayTimeSlider.setBounds( mFeedbackSlider.getX() + space_between_columns, mDryWetSlider.getY() , slider_width , slider_height );
    mDelayTimeLabel.setBounds( mDelayTimeSlider.getX() + label_x_dif, mDelayTimeSlider.getY() - label_y_dif, label_width, label_height );
    
    //  phase offset
    mPhaseOffsetSlider.setBounds( left_margin,  mDryWetSlider.getY() + space_between_rows, slider_width, slider_height );
    mPhaseOffsetLabel.setBounds( mPhaseOffsetSlider.getX() + label_x_dif, mPhaseOffsetSlider.getY() - label_y_dif, label_width, label_height );
    
    // rate
    mRateSlider.setBounds( mPhaseOffsetSlider.getX() + space_between_columns,  mPhaseOffsetSlider.getY(), slider_width, slider_height );
    mRateLabel.setBounds( mRateSlider.getX() + label_x_dif, mRateSlider.getY() - label_y_dif, label_width, label_height );
    
    // depth
    mDepthSlider.setBounds( mRateSlider.getX() + space_between_columns, mPhaseOffsetSlider.getY(), slider_width, slider_height );
    mDepthLabel.setBounds( mDepthSlider.getX() + label_x_dif, mDepthSlider.getY() - label_y_dif, label_width, label_height );
    
    // trem freq
    mTremFreqSlider.setBounds( mDelayTypeBox.getX() + space_between_columns + 20, mGainSlider.getY(), slider_width, slider_height );
    mTremFreqLabel.setBounds( mTremFreqSlider.getX() + label_x_dif, mTremFreqSlider.getY() - label_y_dif, label_width, label_height );
    
    // trem amount
    mTremAmountSlider.setBounds( mTremFreqSlider.getX() + space_between_columns, mGainSlider.getY(), slider_width, slider_height );
    mTremAmountLabel.setBounds( mTremAmountSlider.getX() + label_x_dif, mTremAmountSlider.getY() - label_y_dif, label_width, label_height );
}   // END resized()



                    /*** private member functions ***/
// sliderSetUp()
void MainComponent::sliderSetUp(Slider* slider, float start_value, float range_low_bound, float range_upper_bound, juce::String name, Label* label, int parameterSynth_index, bool standard, int dec_places ) {
    
    slider->setSliderStyle( Slider::SliderStyle::RotaryVerticalDrag );
    slider->setTextBoxStyle( Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 25 );
    slider->setRange( range_low_bound, range_upper_bound );
    slider->setNumDecimalPlacesToDisplay(dec_places);    // setRange() resets this to 7.  This line must come after one above it
    slider->setValue( start_value );
    parametersSynth[parameterSynth_index] = start_value;
    
    AudioAppComponent::addAndMakeVisible( *slider );
    
    // value change response
    if (standard) {
        float* ptr0 = &parametersSynth[ parameterSynth_index ];
        slider->onValueChange =  [ this, ptr0, slider ] { *ptr0 = slider->getValue(); };
    }
    
    // *** fix this before use for DAW production *** TODO
    /* mDryWetSlider.onDragStart = [paramPtr] { dryWetParameter->beginChangeGesture(); };
     mDryWetSlider.onDragEnd = [paramPtr] { dryWetParameter->endChangeGesture(); };  */
    
    // label set up
    label->setText( name, dontSendNotification );
    AudioAppComponent::addAndMakeVisible( *label );
    label->setJustificationType( Justification::centred );
    label->setFont( Font( 13.f, Font::plain ) );
}   // END sliderSetUp()



//  timerCallBack()
void MainComponent::timerCallback() {
    keyboardComponent.grabKeyboardFocus();
    stopTimer();
}   // END timerCallBack()
