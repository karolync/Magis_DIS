/* File written by Jonah Ezekiel (jezekiel@stanford.edu), with many segments copied
from parts of the spinnaker SDK examples. Program includes functions that allow users
to modify features of blackfly model s cameras over software using the Spinnaker SDK
API. File is intended to be used as a dependency for programs a directory up.
        
See function comments for more details. */

//=============================================================================
// Copyright (c) 2001-2019 FLIR Systems, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of FLIR
// Integrated Imaging Solutions, Inc. ("Confidential Information"). You
// shall not disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with FLIR Integrated Imaging Solutions, Inc. (FLIR).
//
// FLIR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. FLIR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <ctime>
#include <vector>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

/* Generic function that allows for the modification of any camera feature variable.
 * Takes as input string representing attribute, camera poitner, nodemap, nodemaptldevice,
 * and string representing value to set attribute to. Correct formatting and names for
 * both attribute and attribute values related to the camera can be found on the camera's
 * technical reference webpage, linked in the project Readme */
int set(gcstring attribute, CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, gcstring value) {
    CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode(attribute);
    if (!IsAvailable(ptrAcquisitionMode)) {
        cout << attribute << " not recognized" << endl;
        return -1;
    }
    if (!IsWritable(ptrAcquisitionMode)) {
        cout << attribute << " not writable" << endl;
        return -1;
    } else {
        CEnumEntryPtr ptrAcquisitionModeType = ptrAcquisitionMode->GetEntryByName(value);
        if(!IsAvailable(ptrAcquisitionModeType) || !IsReadable(ptrAcquisitionModeType)) {
            cout << "Unable to set " << attribute << " to " << value << endl;
            return -1;
        }
        const int64_t acquisitionModeType = ptrAcquisitionModeType->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeType);
        cout << attribute << " set successfully to " << value << endl;
        return 0;
    }   
}

/* Function called by run camera function and prints out info about a camera. Takes
as input nodeMap for the camera */
int PrintDeviceInfo(INodeMap& nodeMap) {
    cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;
    FeatureList_t features;
    const CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
    if (IsAvailable(category) && IsReadable(category)) {
        category->GetFeatures(features);

        for (auto it = features.begin(); it != features.end(); ++it) {
            const CNodePtr pfeatureNode = *it;
            cout << pfeatureNode->GetName() << " : ";
            CValuePtr pValue = static_cast<CValuePtr>(pfeatureNode);
            cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
            cout << endl;
        }
        return 0;
    } else {
        cout << "Device control information not available." << endl;
        return -1;
    }
}

/* Called by run camera function. Modifies ADC bit depth for passed in camera. Takes
 * as input pointer to camera, camras nodeMAP, cameras nodeMapTLDevice, and the adc bit
 * depth, which is either ten, twelve, or fourteen */
int setADCBitDepth(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, int bitDepth) {
    CEnumerationPtr ptrBitDepth = nodeMap.GetNode("AdcBitDepth");
    if (!IsAvailable(ptrBitDepth) || !IsWritable(ptrBitDepth)) {
    cout << "unable to get or write to ADC bit depth node" << endl;
    return -1;
    } else {
    CEnumEntryPtr ptrBitDepthType;
    if (bitDepth == 10) {
        ptrBitDepthType = ptrBitDepth->GetEntryByName("Bit10");
    } else if (bitDepth == 12) {
        ptrBitDepthType = ptrBitDepth->GetEntryByName("Bit12");
    } else if (bitDepth == 14) {
        ptrBitDepthType = ptrBitDepth->GetEntryByName("Bit14");
    } else {
        cout << "Invalid input" << endl;
        return -1;
    }
        if(!IsAvailable(ptrBitDepthType) || !IsReadable(ptrBitDepthType)) {
            cout << "Unable to set ADC bit depth to new value" << endl;
            return -1;
        }
        const int64_t bitDepthType = ptrBitDepthType->GetValue();
        ptrBitDepth->SetIntValue(bitDepthType);
        cout << "Bit depth set to Bit" << bitDepth <<  endl;
        return 0;
    }
}

/* Called by run camera function. Takes as input camera, cameras nodeMap, cameras
 * nodeMapZTLDevice, and an integer representing choice of shutter mode, where zero
 * corresponds to rolling shutter, and 1 to global reset */
int setShutterMode(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, int mode) {
    CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("SensorShutterMode");
    if (!IsAvailable(ptrAcquisitionMode)) {
        cout << "Unable to shutter mode" << endl;
        return -1;
    }
    if (!IsWritable(ptrAcquisitionMode)) {
        cout << "Unable to write shutter mode" << endl;
        return -1;
    } else {
        CEnumEntryPtr ptrAcquisitionModeType;
        if (mode != 1 && mode != 0) {
            cout << "invalid shutter mode input" << endl;
            return -1;
        }
        if (mode == 0) {
            cout << "Setting shutter mode to rolling" << endl;
            ptrAcquisitionModeType = ptrAcquisitionMode->GetEntryByName("Rolling");
        } else {
            cout << "Setting shutter mode to Global Reset" << endl;
            ptrAcquisitionModeType = ptrAcquisitionMode->GetEntryByName("GlobalReset");
        }
        if(!IsAvailable(ptrAcquisitionModeType) || !IsReadable(ptrAcquisitionModeType)) {
            cout << "Unable to set sutter mode" << endl;
            return -1;
        }
        const int64_t acquisitionModeType = ptrAcquisitionModeType->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeType);
        cout << "Shutter mode set successfully" << endl;
        return 0;
    }
}

/* Called by run camera function. Takes as input camera, cameras nodeMap, cameras
 * nodeMapZTLDevice, and an integer representing choice of pixel format, where 8
 * corresponds to mono8, and 16 to mono16 */
int setPixelFormat(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, int mode) {
    CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("PixelFormat");
    if (!IsAvailable(ptrAcquisitionMode)) {
        cout << "Unable to get Pixel Format" << endl;
        return -1;
    }
    if (!IsWritable(ptrAcquisitionMode)) {
        cout << "Unable to write Pixel Format" << endl;
        return -1;
    } else {
        CEnumEntryPtr ptrAcquisitionModeType;
        if (mode != 8 && mode != 16) {
            cout << "invalid Pixel Format input" << endl;
            return -1;
        }
        if (mode == 8) {
            cout << "Setting pixel format to mono8" << endl;
            ptrAcquisitionModeType = ptrAcquisitionMode->GetEntryByName("Mono8");
        } else {
            cout << "Setting pixel format to mono16" << endl;
            ptrAcquisitionModeType = ptrAcquisitionMode->GetEntryByName("Mono16");
        }
        if(!IsAvailable(ptrAcquisitionModeType) || !IsReadable(ptrAcquisitionModeType)) {
            cout << "Unable to set pixel format" << endl;
            return -1;
        }
        const int64_t acquisitionModeType = ptrAcquisitionModeType->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeType);
        cout << "Pixel format set successfully" << endl;
        return 0;
    }
}


/* Called by run camera function. Modifies exposure time for a camera. Takes as input
pointer to the camera, the cameras nodeMap,the cameras nodeMapTLDevice, and the
exposureTime in microseconds */
int setExposureTime(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, int exposureTime) {
    CEnumerationPtr ptrExposureAuto = nodeMap.GetNode("ExposureAuto");
    if (!IsAvailable(ptrExposureAuto) || !IsWritable(ptrExposureAuto)) {
        cout << "Unable to disable automatic exposure (node retrieval). Aborting..." << endl << endl;
        return -1;
    }

    CEnumEntryPtr ptrExposureAutoOff = ptrExposureAuto->GetEntryByName("Off");
    if (!IsAvailable(ptrExposureAutoOff) || !IsReadable(ptrExposureAutoOff)) {
        cout << "Unable to disable automatic exposure (enum entry retrieval). Aborting..." << endl << endl;
        return -1;
    }

    ptrExposureAuto->SetIntValue(ptrExposureAutoOff->GetValue());

    cout << "Automatic exposure disabled..." << endl;

    CFloatPtr ptrExposureTime = nodeMap.GetNode("ExposureTime");
    if (!IsAvailable(ptrExposureTime) || !IsWritable(ptrExposureTime)) {
        cout << "Unable to set exposure time. Aborting..." << endl << endl;
        return -1;
    }

    const double exposureTimeMax = 30000000;
    const double exposureTimeMin = 8;
    if (exposureTime >= exposureTimeMax) {
        exposureTime = exposureTimeMax - 1;
    cout << "Exposure time exceeds maximum" << endl;
    } else if (exposureTime <= exposureTimeMin) {
    exposureTime = exposureTimeMin + 1;
    cout << "Exposure time below minimum" << endl;
    }

    ptrExposureTime->SetValue(exposureTime);

    cout << std::fixed << "Exposure time set to " << exposureTime << " us..." << endl << endl;
    return 0;
}

/* Called by run camera function. Takes as input camera, cameras nodeMap, and cameras
nodeMapTLDevice and sets the acquisition mode to Single Frame */
int setAcquisitionMode(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice) {
    CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
    if (!IsAvailable(ptrAcquisitionMode)) {
        cout << "Unable to get acquisition mode" << endl;
        return -1;
    }
    if (!IsWritable(ptrAcquisitionMode)) {
        cout << "Unable to write acquisition mode" << endl;
        return -1;
    } else {
        CEnumEntryPtr ptrAcquisitionModeType = ptrAcquisitionMode->GetEntryByName("SingleFrame");
        if(!IsAvailable(ptrAcquisitionModeType) || !IsReadable(ptrAcquisitionModeType)) {
            cout << "Unable to set acquisition mode to single frame" << endl;
            return -1;
        }
        const int64_t acquisitionModeType = ptrAcquisitionModeType->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeType);
        cout << "Acquisition mode set to Single Frame" << endl;
        return 0;
    }
}

