/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class AttackEaterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AttackEaterAudioProcessorEditor (AttackEaterAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~AttackEaterAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS_COUNT = 2;
	static const int SCALE = 70;
	static const int LABEL_OFFSET = 25;
	static const int SLIDER_WIDTH = 200;
	static const int HUE = 65;

    //==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    AttackEaterAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS_COUNT] = {};
	juce::Slider m_sliders[N_SLIDERS_COUNT] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS_COUNT] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttackEaterAudioProcessorEditor)
};
