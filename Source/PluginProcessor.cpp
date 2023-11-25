/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EnvelopeFollower::EnvelopeFollower()
{
}

void EnvelopeFollower::setCoef(float attackTimeMs, float releaseTimeMs)
{
	m_AttackCoef = exp(-1000.0f / (attackTimeMs * m_SampleRate));
	m_ReleaseCoef = exp(-1000.0f / (releaseTimeMs * m_SampleRate));
}

float EnvelopeFollower::process(float in)
{
	const float inAbs = fabs(in);
	m_Out1Last = fmaxf(inAbs, m_ReleaseCoef * m_Out1Last + (1.0f - m_ReleaseCoef) * inAbs);
	return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * m_Out1Last;
}

const std::string AttackEaterAudioProcessor::paramsNames[] = { "EatAttack", "Volume" };

//==============================================================================
AttackEaterAudioProcessor::AttackEaterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	eatAttackParameter = apvts.getRawParameterValue(paramsNames[0]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[1]);
}

AttackEaterAudioProcessor::~AttackEaterAudioProcessor()
{
}

//==============================================================================
const juce::String AttackEaterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AttackEaterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AttackEaterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AttackEaterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AttackEaterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AttackEaterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AttackEaterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AttackEaterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AttackEaterAudioProcessor::getProgramName (int index)
{
    return {};
}

void AttackEaterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AttackEaterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	m_envelopeFollower[0].init((int)(sampleRate));
	m_envelopeFollower[1].init((int)(sampleRate));

	m_envelopeFollower[0].setCoef(0.01f, 10.0f);
	m_envelopeFollower[1].setCoef(0.01f, 10.0f);
}

void AttackEaterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AttackEaterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AttackEaterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto eatAttack = powf(eatAttackParameter->load(), 0.1f);
	const auto eatAttackInverse = 1.0f - eatAttack;
	const float THRESHOLD_OFFSET_DB = 6.0f;
	const auto volume = juce::Decibels::decibelsToGain(THRESHOLD_OFFSET_DB * eatAttack + volumeParameter->load());

	// Mics constants
	const float ratio = 1.0f + eatAttack * 7.0f;
	const float R_Inv_minus_One = (1.0f / ratio) - 1.0f;
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	// Get RMS
	const float RMS_SMOOTHING_COEF = 0.45f;
	m_RMSLast = RMS_SMOOTHING_COEF * juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, samples)) + (1.0f - RMS_SMOOTHING_COEF) * m_RMSLast;
	float threshold = m_RMSLast - THRESHOLD_OFFSET_DB;

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		
		// Envelope reference
		auto& envelopeFollower = m_envelopeFollower[channel];

		const float factor = (ratio > 1.0f) ? -1.0f : 1.0f;

		for (int sample = 0; sample < samples; ++sample)
		{
			// Get input
			const float in = channelBuffer[sample];

			// Convert input from gain to dB
			const float indB = juce::Decibels::gainToDecibels(fabsf(in));

			//Get gain reduction, positive values
			const float attenuatedB = (indB >= threshold) ? (indB - threshold) * R_Inv_minus_One : 0.0f;

			// Smooth
			const float smoothdB = factor * envelopeFollower.process(attenuatedB);

			// Apply gain reduction
			const float out = in * juce::Decibels::decibelsToGain(smoothdB);

			// Apply volume, mix and send to output
			channelBuffer[sample] = volume * (eatAttack * out + eatAttackInverse * in);
		}
	}
}

//==============================================================================
bool AttackEaterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AttackEaterAudioProcessor::createEditor()
{
    return new AttackEaterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void AttackEaterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void AttackEaterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AttackEaterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  0.0f,   1.0f, 0.05f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(-24.0f,  24.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AttackEaterAudioProcessor();
}
