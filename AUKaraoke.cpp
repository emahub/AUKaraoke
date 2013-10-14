/*
     File: AUKaraoke.cpp
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
// This file defines the AUKaraoke class, as well as the AUKaraokeEffectKernel
//  helper class.

#include "AUKaraoke.h"
#include <cstring>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUDIOCOMPONENT_ENTRY(AUBaseFactory, AUKaraoke)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The COMPONENT_ENTRY macro is required for the Mac OS X Component Manager to recognize and 
// use the audio unit

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUKaraoke::AUKaraoke
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The constructor for new AUKaraoke audio units
AUKaraoke::AUKaraoke (AudioUnit component) : AUEffectBase (component) {
	CreateElements ();
	Globals () -> UseIndexedParameters (kNumberOfParameters);
    
    SetParameter(kParameter_Pan, kDefaultValue_Pan);
    SetParameter(kParameter_Level, kDefaultValue_Level);
    SetParameter(kParameter_FilterSwitch, kDefaultValue_FilterSwitch);
    SetParameter(kParameter_FilterFrequency, kDefaultValue_FilterFrequency);
    SetParameter(kParameter_FilterQ, kDefaultValue_FilterQ);
    

	// Also during instantiation, sets the preset menu to indicate the default preset,
	//	which corresponds to the default parameters. It's possible to set this so a
	//	fresh audio unit indicates the wrong preset, so be careful to get it right.
	SetAFactoryPresetAsCurrent (
		kPresets [kPreset_Default]
	);
    
    
    filter = new BiquadFilter[4];
    filter[0] = *new BiquadFilter(kBiquadFilter_Notch, GetSampleRate());
    filter[1] = *new BiquadFilter(kBiquadFilter_Notch, GetSampleRate());
    filter[2] = *new BiquadFilter(kBiquadFilter_BPF, GetSampleRate());
    filter[3] = *new BiquadFilter(kBiquadFilter_BPF, GetSampleRate());
    
    #ifdef DEBUG
        std::string bPath, bFullFileName;
        bPath = getenv("HOME");
        if (!bPath.empty()) {
            bPath = bPath + "/Desktop/";
            bFullFileName = bPath + "AUKaraoke_baseDebug.log";
        } else {
            bFullFileName = "AUKaraoke_baseDebug.log";
        }
	
        baseDebugFile.open(bFullFileName.c_str());
        DEBUGLOG_B("Plug-in constructor invoked with parameters:" << std::endl);
    #endif
    
	#if AU_DEBUG_DISPATCHER
		mDebugDispatcher = new AUDebugDispatcher (this);
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUKaraoke::AUKaraoke
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The destructor for new AUKaraoke audio units
AUKaraoke::~AUKaraoke () {
    delete[] filter;
}


#pragma mark ____Parameters

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUKaraoke::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Called by the audio unit's view; provides information needed for the view to display the
//  audio unit's parameters
ComponentResult AUKaraoke::GetParameterInfo (
		AudioUnitScope			inScope,
		AudioUnitParameterID	inParameterID,
		AudioUnitParameterInfo	&outParameterInfo
) {
	ComponentResult result = noErr;

	outParameterInfo.flags = 	  
		kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;
    
	if (inScope == kAudioUnitScope_Global) {
        switch (inParameterID) {                                
            case kParameter_Pan:
				AUBase::FillInParameterName (outParameterInfo, kParamName_Pan, false);
				outParameterInfo.unit			= kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue		= kMinimumValue_Pan;
				outParameterInfo.maxValue		= kMaximumValue_Pan;
				outParameterInfo.defaultValue	= kDefaultValue_Pan;
				break;
                
            case kParameter_Level:
				AUBase::FillInParameterName (outParameterInfo, kParamName_Level, false);
				outParameterInfo.unit			= kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue		= kMinimumValue_Level;
				outParameterInfo.maxValue		= kMaximumValue_Level;
				outParameterInfo.defaultValue	= kDefaultValue_Level;
				break;
                
            case kParameter_FilterSwitch:
                AUBase::FillInParameterName (outParameterInfo, kParamName_FilterSwitch, false);
				outParameterInfo.unit			= kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue		= 0;    // must be written
				outParameterInfo.maxValue		= 1;    // must be written
				outParameterInfo.defaultValue	= kDefaultValue_FilterSwitch;
				break;
                
            case kParameter_FilterFrequency:
				AUBase::FillInParameterName (outParameterInfo, kParamName_FilterFrequency, false);
				outParameterInfo.unit			= kAudioUnitParameterUnit_Hertz;
				outParameterInfo.minValue		= kMinimumValue_FilterFrequency;
				outParameterInfo.maxValue		= kMaximumValue_FilterFrequency;
				outParameterInfo.defaultValue	= kDefaultValue_FilterFrequency;
				break;
                
            case kParameter_FilterQ:
				AUBase::FillInParameterName (outParameterInfo, kParamName_FilterQ, false);
				outParameterInfo.unit			= kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue		= kMinimumValue_FilterQ;
				outParameterInfo.maxValue		= kMaximumValue_FilterQ;
				outParameterInfo.defaultValue	= kDefaultValue_FilterQ;
				break;
                
			default:
                DEBUGLOG_B("GetParameterInfo inParameterID: " << inParameterID);
				result = kAudioUnitErr_InvalidParameter;
				break;
		}
	} else {
        result = kAudioUnitErr_InvalidParameter;
    }
	return result;
}

#pragma mark ____Properties

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUKaraoke::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult AUKaraoke::GetPropertyInfo (
	// This audio unit doesn't define any custom properties, so it uses this generic code for
	// this method.
	AudioUnitPropertyID	inID,
	AudioUnitScope		inScope,
	AudioUnitElement	inElement,
	UInt32				&outDataSize,
	Boolean				&outWritable
) {
	return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUKaraoke::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult AUKaraoke::GetProperty (
	// This audio unit doesn't define any custom properties, so it uses this generic code for
	// this method.
	AudioUnitPropertyID inID,
	AudioUnitScope 		inScope,
	AudioUnitElement 	inElement,
	void				*outData
) {
	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}

#pragma mark ____Factory Presets

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUKaraoke::GetPresets
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// For users to be able to use the factory presets you define, you must add a generic 
// implementation of the GetPresets method. The code here works for any audio unit that can 
// support factory presets.

// The GetPresets method accepts a single parameter, a pointer to a CFArrayRef object. This 
// object holds the factory presets array generated by this method. The array contains just 
// factory preset numbers and names. The host application uses this array to set up its 
// factory presets menu and when calling the NewFactoryPresetSet method.

ComponentResult AUKaraoke::GetPresets (
	CFArrayRef	*outData
) const {

	// Checks whether factory presets are implemented for this audio unit.
	if (outData == NULL) return noErr;
	
	// Instantiates a mutable Core Foundation array to hold the factory presets.
	CFMutableArrayRef presetsArray = CFArrayCreateMutable (
		NULL,
		kNumberPresets,
		NULL
	);
	
	// Fills the factory presets array with values from the definitions in the AUKaraoke.h 
	// file.
	for (int i = 0; i < kNumberPresets; ++i) {
		CFArrayAppendValue (
			presetsArray,
			&kPresets [i]
		);
    }
    
	// Stores the factory presets array at the outData location.
	*outData = (CFArrayRef) presetsArray;
	return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUKaraoke::NewFactoryPresetSet
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The NewFactoryPresetSet method defines all the factory presets for an audio unit. Basically, 
// for each preset, it invokes a series of SetParameter calls.

// This method takes a single argument of type AUPreset, a structure containing a factory
//  preset name and number.
OSStatus AUKaraoke::NewFactoryPresetSet (const AUPreset &inNewFactoryPreset) {
	// Gets the number of the desired factory preset.
	SInt32 chosenPreset = inNewFactoryPreset.presetNumber;
	
	if (
		// Tests whether the desired factory preset is defined.
		chosenPreset == kPreset_Normal ||
        chosenPreset == kPreset_CenterRemove ||
		chosenPreset == kPreset_Karaoke
    ) {
		// This 'for' loop, and the 'if' statement that follows it, allow for noncontiguous preset 
		// numbers.
		for (int i = 0; i < kNumberPresets; ++i) {
			if (chosenPreset == kPresets[i].presetNumber) {

				//Selects the appropriate case statement based on the factory preset number.
				switch (chosenPreset) {
					case kPreset_Normal:
                        SetParameter(kParameter_Pan, kParameter_Preset_Pan_Normal);
                        SetParameter(kParameter_Level, kParameter_Preset_Level_Normal);
                        SetParameter(kParameter_FilterSwitch, kParameter_Preset_FilterSwitch_Normal);
                        SetParameter(kParameter_FilterFrequency, kParameter_Preset_FilterFrequency_Normal);
                        SetParameter(kParameter_FilterQ, kParameter_Preset_FilterQ_Normal);
                        break;
                        
					case kPreset_CenterRemove:
                        SetParameter(kParameter_Pan, kParameter_Preset_Pan_CenterRemove);
                        SetParameter(kParameter_Level, kParameter_Preset_Level_CenterRemove);
                        SetParameter(kParameter_FilterSwitch, kParameter_Preset_FilterSwitch_CenterRemove);
                        SetParameter(kParameter_FilterFrequency, kParameter_Preset_FilterFrequency_CenterRemove);
                        SetParameter(kParameter_FilterQ, kParameter_Preset_FilterQ_CenterRemove);
						break;
                        
					case kPreset_Karaoke:
                        SetParameter(kParameter_Pan, kParameter_Preset_Pan_Karaoke);
                        SetParameter(kParameter_Level, kParameter_Preset_Level_Karaoke);
                        SetParameter(kParameter_FilterSwitch, kParameter_Preset_FilterSwitch_Karaoke);
                        SetParameter(kParameter_FilterFrequency, kParameter_Preset_FilterFrequency_Karaoke);
                        SetParameter(kParameter_FilterQ, kParameter_Preset_FilterQ_Karaoke);
						break;

				}
				
				// Updates the preset menu in the generic view to display the new factory preset.
				SetAFactoryPresetAsCurrent (
					kPresets [i]
				);
				return noErr;
			}
		}
	}
	return kAudioUnitErr_InvalidProperty;
}



#pragma mark ____AUKaraokeEffectKernel

//--------------------------------------------------------------------------------
UInt32 AUKaraoke::SupportedNumChannels(const AUChannelInfo ** outChannelInfo)
{
    static AUChannelInfo plugChannelInfo[] = { {kNumInputs, kNumOutputs} };
    
    if (outChannelInfo != NULL)
        *outChannelInfo = plugChannelInfo;
    
    return sizeof(plugChannelInfo) / sizeof(plugChannelInfo[0]);
}

//--------------------------------------------------------------------------------
OSStatus AUKaraoke::ProcessBufferLists(AudioUnitRenderActionFlags & ioActionFlags,
                                                        const AudioBufferList & inBuffer, AudioBufferList & outBuffer, UInt32 inFramesToProcess)
{
    const float * in[kNumInputs] =
        {(float*)(inBuffer.mBuffers[0].mData), (float*)(inBuffer.mBuffers[1].mData)};
    
    float * out[kNumOutputs] =
        {(float*)(outBuffer.mBuffers[0].mData), (float*)(outBuffer.mBuffers[1].mData)};
    
    float param_L2L = 1.0f;
    float param_L2R = -1.0f;
    float param_R2L = -1.0f;
    float param_R2R = 1.0f;
    float param_Pan = GetParameter(kParameter_Pan);
    float param_Level = GetParameter(kParameter_Level);
    int param_FilterSwitch = GetParameter(kParameter_FilterSwitch);
    int param_FilterFrequency = GetParameter(kParameter_FilterFrequency);
    float param_FilterQ = GetParameter(kParameter_FilterQ);
    
    
    if(param_Pan < 0.0f){
        param_L2L = 1 / pow(1/ (1+param_Pan), 2);
        param_L2R = -param_L2L;
    } else if(param_Pan > 0.0f){
        param_R2R = 1 / pow(1/ (1-param_Pan), 2);
        param_R2L = -param_R2R;
    }
    
    param_L2L = this->ofMap(param_Level, -1.0f, 0, param_L2L, 1.0f, false);
    param_L2R = this->ofMap(param_Level, -1.0f, 0, param_L2R, 0, false);
    param_R2L = this->ofMap(param_Level, -1.0f, 0, param_R2L, 0, false);
    param_R2R = this->ofMap(param_Level, -1.0f, 0, param_R2R, 1.0f, false);
    
    
    //DEBUGLOG_B("Pan:" << param_Pan << " Level:"<<param_Level <<
    //           " L2L:"<<param_L2L << " L2R:"<<param_L2R <<
    //           " R2L:"<<param_R2L << " R2R:"<<param_R2R << std::endl);
    

    for (UInt32 samp=0; samp < inFramesToProcess; samp++)
    {
        float s[2] = {in[0][samp], in[1][samp]};
        
        if(param_FilterSwitch == 1){
            filter[0].set(param_FilterFrequency, param_FilterQ);
            filter[1].set(param_FilterFrequency, param_FilterQ);
            filter[2].set(param_FilterFrequency, param_FilterQ);
            filter[3].set(param_FilterFrequency, param_FilterQ);
            
            out[0][samp] = filter[0].get(s[0]) +
            filter[2].get(param_L2L * s[0] + param_R2L * s[1]);
            out[1][samp] = filter[1].get(s[1]) +
            filter[3].get(param_L2R * s[0] + param_R2R * s[1]);
            
        } else {
            out[0][samp] = param_L2L * s[0] + param_R2L * s[1];
            out[1][samp] = param_L2R * s[0] + param_R2R * s[1];
        }
        
    }
    
    return noErr;
}


//--------------------------------------------------------------------------------
float AUKaraoke::ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp) {
    if (fabs(inputMin - inputMax) < FLT_EPSILON){
        DEBUGLOG_B("ofMap: avoiding possible divide by zero, check inputMin and inputMax" << std::endl);
        return outputMin;
    } else {
        float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
        if( clamp ){
            if(outputMax < outputMin){
                if( outVal < outputMax )outVal = outputMax;
                else if( outVal > outputMin )outVal = outputMin;
            }else{
                if( outVal > outputMax )outVal = outputMax;
                else if( outVal < outputMin )outVal = outputMin;
            }
        }
        return outVal;
    }
}
