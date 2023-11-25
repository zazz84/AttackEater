/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AttackEaterAudioProcessorEditor::AttackEaterAudioProcessorEditor (AttackEaterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
	for (int i = 0; i < N_SLIDERS_COUNT; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];

		//Lable
		label.setText(AttackEaterAudioProcessor::paramsNames[i], juce::dontSendNotification);
		label.setFont(juce::Font(24.0f * 0.01f * SCALE, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);

		//Slider
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, AttackEaterAudioProcessor::paramsNames[i], slider));
	}

	setSize((int)(200.0f * 0.01f * SCALE * N_SLIDERS_COUNT), (int)(200.0f * 0.01f * SCALE));
}

AttackEaterAudioProcessorEditor::~AttackEaterAudioProcessorEditor()
{
}

//==============================================================================
void AttackEaterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colours::darkslateblue);
}

void AttackEaterAudioProcessorEditor::resized()
{
	int width = getWidth() / N_SLIDERS_COUNT;
	int height = getHeight();
	
	// Sliders + Menus
	juce::Rectangle<int> rectangles[N_SLIDERS_COUNT];

	for (int i = 0; i < N_SLIDERS_COUNT; ++i)
	{
		rectangles[i].setSize(width, height);
		rectangles[i].setPosition(i * width, 0);
		m_sliders[i].setBounds(rectangles[i]);

		rectangles[i].removeFromBottom((int)(20.0f * 0.01f * SCALE));
		m_labels[i].setBounds(rectangles[i]);
	}
}