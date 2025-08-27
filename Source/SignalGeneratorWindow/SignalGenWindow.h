#pragma once

#include <JuceHeader.h>
#include "SignalGeneratorComponent.h"

class SignalGenWindow : public juce::DocumentWindow
{
public:
    explicit SignalGenWindow(SignalGenAudioEngine* sharedEngine = nullptr)
        : juce::DocumentWindow("Signal Generator",
                               juce::Colours::lightgrey,
                               juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        // When integrated, UI should not own audio devices; main app drives audio
        setContentOwned(new SignalGeneratorComponent(sharedEngine, false), true);
        setResizable(true, true);
        centreWithSize(760, 620);
    }

    std::function<void()> onClose;

    void closeButtonPressed() override
    {
        setVisible(false);
        // Keep the instance so it can be re-shown later
        if (onClose)
            onClose();
    }
};


