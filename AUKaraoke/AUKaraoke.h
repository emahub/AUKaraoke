/*
     File: AUKaraoke.h
 Abstract: AUKaraoke.h
  Version: 1.1
*/

#include "AUEffectBase.h"
#include "AUKaraokeVersion.h"
#include "BiquadFilter.h"
#include <cmath>
#include <cstdlib>

#if AU_DEBUG_DISPATCHER
#include "AUDebugDispatcher.h"
#endif

#ifndef __AUKaraoke_h__
#define __AUKaraoke_h__

#ifdef DEBUG
#include <fstream>
#include <ctime>
#endif

// two debug macros (one for the "base" class, one for "kernel") defined
#ifdef DEBUG
#define DEBUGLOG_B(x) \
  if (baseDebugFile.is_open()) baseDebugFile << "\t" << x
#define DEBUGLOG_K(x) \
  if (kernelDebugFile.is_open()) kernelDebugFile << "\t" << x
#else
#define DEBUGLOG_B(x)
#define DEBUGLOG_K(x)
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Constants for parameters and  factory presets
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUKaraoke Parameter Constants

const SInt16 kNumInputs = 2;
const SInt16 kNumOutputs = 2;

static CFStringRef kParamName_Pan = CFSTR("Pan");
static const float kDefaultValue_Pan = 0.0f;
static const float kMinimumValue_Pan = -1.0f;
static const float kMaximumValue_Pan = 1.0f;

static CFStringRef kParamName_Level = CFSTR("Level");
static const float kDefaultValue_Level = 0.0f;
static const float kMinimumValue_Level = -1.0f;
static const float kMaximumValue_Level = 0.0f;

static CFStringRef kParamName_FilterSwitch = CFSTR("Filter Switch");
static const bool kDefaultValue_FilterSwitch = false;

static CFStringRef kParamName_FilterFrequency = CFSTR("Cutoff Frequency");
static const int kDefaultValue_FilterFrequency = 1000;
static const int kMinimumValue_FilterFrequency = 10;
static const int kMaximumValue_FilterFrequency = 10000;

static CFStringRef kParamName_FilterQ = CFSTR("Q");
static const float kDefaultValue_FilterQ = 1.0f;
static const float kMinimumValue_FilterQ = 0.001f;
static const float kMaximumValue_FilterQ = 1.0f;

// Defines constants for identifying the parameters; defines the total number
//  of parameters.
enum {
  kParameter_Pan = 0,
  kParameter_Level = 1,
  kParameter_FilterSwitch = 2,
  kParameter_FilterFrequency = 3,
  kParameter_FilterQ = 4,
  kNumberOfParameters = 5
};

#pragma mark ____AUKaraoke Factory Preset Constants

static const float kParameter_Preset_Pan_Normal = 0.0f;
static const float kParameter_Preset_Level_Normal = 0.0f;
static const int kParameter_Preset_FilterSwitch_Normal = 0;
static const int kParameter_Preset_FilterFrequency_Normal = 10.0f;
static const float kParameter_Preset_FilterQ_Normal = 1.0f;

static const float kParameter_Preset_Pan_CenterRemove = 0.0f;
static const float kParameter_Preset_Level_CenterRemove = -1.0f;
static const int kParameter_Preset_FilterSwitch_CenterRemove = 0;
static const int kParameter_Preset_FilterFrequency_CenterRemove = 10.0f;
static const float kParameter_Preset_FilterQ_CenterRemove = 1.0f;

static const float kParameter_Preset_Pan_Karaoke = 0.0f;
static const float kParameter_Preset_Level_Karaoke = -1.0f;
static const int kParameter_Preset_FilterSwitch_Karaoke = 1;
static const int kParameter_Preset_FilterFrequency_Karaoke = 2000.0f;
static const float kParameter_Preset_FilterQ_Karaoke = 0.1f;

enum {
  kPreset_Normal = 0,
  kPreset_CenterRemove = 1,
  kPreset_Karaoke = 2,
  kNumberPresets = 3
};

// Defines an array containing two Core Foundation string objects. The objects
// contain
//  values for the menu items in the user interface corresponding to the factory
// presets.
static AUPreset kPresets[kNumberPresets] = {
    {kPreset_Normal, CFSTR("Normal")},
    {kPreset_CenterRemove, CFSTR("Center Pan Remover")},
    {kPreset_Karaoke, CFSTR("Karaoke")}};

// Defines a constant representing the default factory preset
static const int kPreset_Default = kPreset_Normal;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AUKaraoke class
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUKaraoke
class AUKaraoke : public AUEffectBase {

 public:
  AUKaraoke(AudioUnit component);

#if AU_DEBUG_DISPATCHER
  virtual ~AUKaraoke() { delete mDebugDispatcher; }
#endif

  virtual ComponentResult GetParameterValueStrings(
      AudioUnitScope inScope, AudioUnitParameterID inParameterID,
      CFArrayRef *outStrings);

  virtual ComponentResult GetParameterInfo(
      AudioUnitScope inScope, AudioUnitParameterID inParameterID,
      AudioUnitParameterInfo &outParameterInfo);

  virtual ComponentResult GetPropertyInfo(AudioUnitPropertyID inID,
                                          AudioUnitScope inScope,
                                          AudioUnitElement inElement,
                                          UInt32 &outDataSize,
                                          Boolean &outWritable);

  virtual ComponentResult GetProperty(AudioUnitPropertyID inID,
                                      AudioUnitScope inScope,
                                      AudioUnitElement inElement,
                                      void *outData);

  // report that the audio unit supports the
  //	kAudioUnitProperty_TailTime property
  virtual bool SupportsTail() { return true; }

  // provide the audio unit version information
  virtual ComponentResult Version() { return kAUKaraokeVersion; }

  // Declaration for the GetPresets method (for setting up the factory presets),
  //  overriding the method from the AUBase superclass.
  virtual ComponentResult GetPresets(CFArrayRef *outData) const;

  // Declaration for the NewFactoryPresetSet method (for setting a factory
  // preset
  //  when requested by the host application), overriding the method from the
  //  AUBase superclass.
  virtual OSStatus NewFactoryPresetSet(const AUPreset &inNewFactoryPreset);

  // added for stereo
  virtual UInt32 SupportedNumChannels(const AUChannelInfo **outChannelInfo);
  virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags &ioActionFlags,
                                      const AudioBufferList &inBuffer,
                                      AudioBufferList &outBuffer,
                                      UInt32 inFramesToProcess);

  // others added
  BiquadFilter *filter;

  float ofMap(float value, float inputMin, float inputMax, float outputMin,
              float outputMax, bool clamp);

 protected:
#ifdef DEBUG
  std::ofstream baseDebugFile;
#endif
};

#endif
