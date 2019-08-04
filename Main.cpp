#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"

//==============================================================================
class my_synthApplication  : public JUCEApplication
{
public:
    //==============================================================================
    my_synthApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName(), new MainComponent(), *this));
    }

    void shutdown() override
    {
        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override {}

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (const String& name, Component* c, JUCEApplication& a)
        : DocumentWindow (name, Desktop::getInstance().getDefaultLookAndFeel()
                          .findColour (ResizableWindow::backgroundColourId),
                          DocumentWindow::allButtons), app (a)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }


    private:
        JUCEApplication& app;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (my_synthApplication)
