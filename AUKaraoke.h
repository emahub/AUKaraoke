/*
     File: AUKaraoke.h
 Abstract: AUKaraoke.h
  Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2012 Apple Inc. All Rights Reserved.
 
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
    #define DEBUGLOG_B(x) if(baseDebugFile.is_open()) baseDebugFile << "\t" << x
    #define DEBUGLOG_K(x) if(kernelDebugFile.is_open()) kernelDebugFile << "\t" << x
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

static CFStringRef kParamName_Pan       = CFSTR("Pan");
static const float kDefaultValue_Pan    = 0.0f;
static const float kMinimumValue_Pan    = -1.0f;
static const float kMaximumValue_Pan    = 1.0f;

static CFStringRef kParamName_Level     = CFSTR("Level");
static const float kDefaultValue_Level  = 0.0f;
static const float kMinimumValue_Level  = -1.0f;
static const float kMaximumValue_Level  = 0.0f;

static CFStringRef kParamName_FilterSwitch  = CFSTR("Filter Switch");
static const bool kDefaultValue_FilterSwitch    = false;

static CFStringRef kParamName_FilterFrequency   = CFSTR("Cutoff Frequency");
static const int kDefaultValue_FilterFrequency  = 1000;
static const int kMinimumValue_FilterFrequency  = 10;
static const int kMaximumValue_FilterFrequency  = 10000;

static CFStringRef kParamName_FilterQ   = CFSTR("Q");
static const float kDefaultValue_FilterQ = 1.0f;
static const float kMinimumValue_FilterQ  = 0.001f;
static const float kMaximumValue_FilterQ  = 1.0f;

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
    kPreset_Karaoke	= 2,
	kNumberPresets	= 3
};

// Defines an array containing two Core Foundation string objects. The objects contain 
//  values for the menu items in the user interface corresponding to the factory presets.
static AUPreset kPresets [kNumberPresets] = {
    {kPreset_Normal, CFSTR ("Normal")},
	{kPreset_CenterRemove, CFSTR ("Center Pan Remover")},
	{kPreset_Karaoke, CFSTR ("Karaoke")}
};

// Defines a constant representing the default factory preset, in this case the 
//  "Slow & Gentle" preset.
static const int kPreset_Default = kPreset_Normal;



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AUKaraoke class
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AUKaraoke
class AUKaraoke : public AUEffectBase {

public:
	AUKaraoke (AudioUnit component);
    ~AUKaraoke ();
	
#if AU_DEBUG_DISPATCHER
	virtual ~AUKaraoke () { DEBUGLOG_B("AU plug-in destroyed" << '\n'); delete mDebugDispatcher; }
#endif
    
	virtual	ComponentResult GetParameterInfo (
		AudioUnitScope			inScope,
		AudioUnitParameterID	inParameterID,
		AudioUnitParameterInfo	&outParameterInfo
	);
    
	virtual ComponentResult GetPropertyInfo (
		AudioUnitPropertyID		inID,
		AudioUnitScope			inScope,
		AudioUnitElement		inElement,
		UInt32					&outDataSize,
		Boolean					&outWritable
	);
	
	virtual ComponentResult GetProperty (
		AudioUnitPropertyID		inID,
		AudioUnitScope			inScope,
		AudioUnitElement		inElement,
		void					*outData
	);
	
 	// report that the audio unit supports the 
	//	kAudioUnitProperty_TailTime property
	virtual	bool SupportsTail () {return true;}
	
	// provide the audio unit version information
	virtual ComponentResult	Version () {return kAUKaraokeVersion;}
    
    // added for stereo
    virtual UInt32 SupportedNumChannels(const AUChannelInfo ** outChannelInfo);
    virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags, const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess);
    

    // Declaration for the GetPresets method (for setting up the factory presets),
	//  overriding the method from the AUBase superclass.
    virtual ComponentResult	GetPresets (
		CFArrayRef	*outData
	) const;
	
	// Declaration for the NewFactoryPresetSet method (for setting a factory preset 
	//  when requested by the host application), overriding the method from the 
	//  AUBase superclass.
    virtual OSStatus NewFactoryPresetSet (
		const AUPreset	&inNewFactoryPreset
	);
    
    BiquadFilter* filter;
    
    float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp);

protected:
#ifdef DEBUG
	std::ofstream baseDebugFile;
#endif
    
};

#endif
