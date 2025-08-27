#include <JuceHeader.h>
#include "MainComponent.h"

// Main window class
class AudioStreamFilterApp : public juce::JUCEApplication
{
public:
    AudioStreamFilterApp() {}

    const juce::String getApplicationName() override { return "AudioEffectTools"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& /*commandLine*/) override
    {
        mainWindow.reset(new MainWindow(getApplicationName(), new MainComponent()));
    }

    void shutdown() override
    {
        mainWindow = nullptr; // Clean up
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name, juce::Component* component)
            : DocumentWindow(name+"[" + juce::String(__DATE__) + " " + juce::String(__TIME__) + "]", juce::Colours::lightgrey, DocumentWindow::allButtons)
        {
            // setUsingNativeTitleBar(true);
            // setContentOwned(component, true);
            // centreWithSize(getWidth(), getHeight());
            // setVisible(true);
            setUsingNativeTitleBar(true);
            setContentOwned(component, true);
            
            // Get main display information
            auto displays = juce::Desktop::getInstance().getDisplays();
            auto mainDisplay = displays.getPrimaryDisplay();
            auto screenArea = mainDisplay->userArea;
            
            // Set appropriate window size
            int windowWidth = juce::jmin(1200, screenArea.getWidth() - 100);
            int windowHeight = juce::jmin(800, screenArea.getHeight() - 100);
            
            // Force center and ensure it's on screen
            centreWithSize(windowWidth, windowHeight);
            
            // Ensure window is fully on screen
            auto bounds = getBounds();
            bounds = bounds.constrainedWithin(screenArea);
            setBounds(bounds);
            
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

// JUCE application entry point
START_JUCE_APPLICATION(AudioStreamFilterApp)