/* File written by Jonah Ezekiel (jezekiel@stanford.edu), with many segments copied
from parts of the spinnaker SDK examples. Program allows user to test out key
functionality of working with the Spinnaker SDK API to interface with Flir
Blackfly s cameras over software. In running takes no inputs, but generates a
CLI in order to trigger the camera to take photos, and between photos optionally
modify the exposure time. Photo files stored in /magis/data/DIS/lab_images.

Call order in case of one camera detected:

main calls runSingleCamera
	runSingleCamera calls PrintDeviceInfo
	runSingleCamera calls setAcquisitionMode
	runSingleCamera calls getImage or setExposureTime
		
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

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

#ifdef _DEBUG
// Disables heartbeat on GEV cameras so debugging does not incur timeout errors
int DisableHeartbeat(INodeMap& nodeMap, INodeMap& nodeMapTLDevice)
{
    cout << "Checking device type to see if we need to disable the camera's heartbeat..." << endl << endl;

    //
    // Write to boolean node controlling the camera's heartbeat
    //
    // *** NOTES ***
    // This applies only to GEV cameras and only applies when in DEBUG mode.
    // GEV cameras have a heartbeat built in, but when debugging applications the
    // camera may time out due to its heartbeat. Disabling the heartbeat prevents
    // this timeout from occurring, enabling us to continue with any necessary debugging.
    // This procedure does not affect other types of cameras and will prematurely exit
    // if it determines the device in question is not a GEV camera.
    //
    // *** LATER ***
    // Since we only disable the heartbeat on GEV cameras during debug mode, it is better
    // to power cycle the camera after debugging. A power cycle will reset the camera
    // to its default settings.
    //
    CEnumerationPtr ptrDeviceType = nodeMapTLDevice.GetNode("DeviceType");
    if (!IsAvailable(ptrDeviceType) || !IsReadable(ptrDeviceType))
    {
        cout << "Error with reading the device's type. Aborting..." << endl << endl;
        return -1;
    }
    else
    {
        if (ptrDeviceType->GetIntValue() == DeviceType_GigEVision)
        {
            cout << "Working with a GigE camera. Attempting to disable heartbeat before continuing..." << endl << endl;
            CBooleanPtr ptrDeviceHeartbeat = nodeMap.GetNode("GevGVCPHeartbeatDisable");
            if (!IsAvailable(ptrDeviceHeartbeat) || !IsWritable(ptrDeviceHeartbeat))
            {
                cout << "Unable to disable heartbeat on camera. Continuing with execution as this may be non-fatal..."
                     << endl
                     << endl;
            }
            else
            {
                ptrDeviceHeartbeat->SetValue(true);
                cout << "WARNING: Heartbeat on GigE camera disabled for the rest of Debug Mode." << endl;
                cout << "         Power cycle camera when done debugging to re-enable the heartbeat..." << endl << endl;
            }
        }
        else
        {
            cout << "Camera does not use GigE interface. Resuming normal execution..." << endl << endl;
        }
    }
    return 0;
}
#endif

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

/* Called by run camera function. Modifies ADC bit depth for passed in camera. Takes as input pointer to camera, camras nodeMAP, cameras nodeMapTLDevice, and the adc bit depth, which is either ten, twelve, or fourteen */
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

/* Called by run camera function. Takes as input camera, camera nodeMap, and cameras
nodeMapTLDevice. Acquires a single image and saves this image in the specified directory
with the name Acquisition-(Date & time).jpg. Works under assumption that camera is in
Acquisition mode. */
int getImage(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice) {
    cout << "Getting Image" << endl;
    ImagePtr pResultImage = pCam->GetNextImage(1000);
    if (pResultImage->IsIncomplete()) {
        cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << endl;
        return -1;
    } else {
        //optionally convert image at this point to different format
        ostringstream filename;
	time_t now = time(0);
	filename << "/home/pi/magis/data/DIS/lab_images/";
        filename << "Acquisition-";
       	filename << ctime(&now);
        filename << ".png";
        cout << "Saving image" << endl;
        pResultImage->Save(filename.str().c_str());
        cout << "Image saved" << endl;
        pResultImage->Release();
        return 0;
    }
}

/* Function called by main in case where only one camera connected. Function calls PrintDeviceInfo
and setAcquisitionMode, setting up the camera and generates cli allowing user to take photos via
function getImages and change exposure time via function setExposureTime. */
int runSingleCamera(CameraPtr pCam) {
    INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
    PrintDeviceInfo(nodeMapTLDevice);
    pCam->Init();
    INodeMap& nodeMap = pCam->GetNodeMap();
    setAcquisitionMode(pCam, nodeMap, nodeMapTLDevice);

#ifdef _DEBUG
        cout << endl << endl << "*** DEBUG ***" << endl << endl;

        // If using a GEV camera and debugging, should disable heartbeat first to prevent further issues
        if (DisableHeartbeat(nodeMap, nodeMapTLDevice) != 0)
        {
            return -1;
        }

        cout << endl << endl << "*** END OF DEBUG ***" << endl << endl;
#endif

    cout << "camera in Acquisition Mode" << endl; 
    //modify device settings here 
    while (true) {
    	pCam->BeginAcquisition();
	cout << "press c to take a photo, e to modify exposure time, b to modify adc bit depth, and x to quit" << endl;
        char input;
        cin >> input;
        cout << "Input: " << input << endl;
        if (input == 'c') {
            getImage(pCam, nodeMap, nodeMapTLDevice);
            cout << "Image acquired" << endl;
	    pCam->EndAcquisition();
        } else if (input == 'x') {
	    pCam->EndAcquisition();
	    break;
        } else if (input == 'e') {
            pCam->EndAcquisition();
	    int exposureTime;
            cout << "new exposure time (microseconds):" << endl;
            cin >> exposureTime;
            setExposureTime(pCam, nodeMap, nodeMapTLDevice, exposureTime);
        } else if (input == 'b') {
            pCam->EndAcquisition();
	    int bitDepth;
	    cout << "bit10: 10, bit12: 12, bit14: 14" << endl;
	    cin >> bitDepth;
	    setADCBitDepth(pCam, nodeMap, nodeMapTLDevice, bitDepth);
	}
    }
    pCam->DeInit();
    return 0;
}

/* Main function takes as input no arguments. Determines number of cameras and calls corresponding run
camera(s) function. */
int main(int /*argc*/, char** /*argv*/) {
    FILE* tempFile = fopen("test.txt", "w+"); // checks if we have permission to write to the current folder
    if (tempFile == nullptr)
    {
        cout << "Permission denied to write to the current foldr. Press Enter to exit" << endl;
        getchar();
        return -1;
    }
    fclose(tempFile);
    remove("test.txt");

    // Print application build information
    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;

    // Retrieve singleton reference to system object
    SystemPtr system = System::GetInstance();

    // Print out current library version
    const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();
    cout << "Spinnaker library version: " << spinnakerLibraryVersion.major << "." << spinnakerLibraryVersion.minor
         << "." << spinnakerLibraryVersion.type << "." << spinnakerLibraryVersion.build << endl
         << endl;

    // Retrieve list of cameras from the system
    CameraList camList = system->GetCameras();

    const unsigned int numCameras = camList.GetSize();

    cout << "Number of cameras detected: " << numCameras << endl << endl;

    // Finish if there are no cameras
    if (numCameras == 0)
    {
        // Clear camera list before releasing system
        camList.Clear();

        // Release system
        system->ReleaseInstance();

        cout << "Not enough cameras! Press Enter to exit." << endl;
        getchar();
        return -1;
    } else if (numCameras == 1) {
        cout << "One camera detected" << endl;
        
        CameraPtr pCam = camList.GetByIndex(0);
        runSingleCamera(pCam);

        pCam = nullptr;
    } else if (numCameras == 2) {
        cout << "Two cameras detected" << endl;
        
        CameraPtr pCam1 = camList.GetByIndex(0);
        CameraPtr pCam2 = camList.GetByIndex(1);



        pCam1 = nullptr;
        pCam2 = nullptr;
    }
    camList.Clear();
    system->ReleaseInstance();
    return 0;
}
