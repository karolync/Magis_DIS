/* DO NOT USE FILE. THIS FILE IS STILL IN PRODUCTION


File written by Jonah Ezekiel (jezekiel@stanford.edu), with many segments copied
from parts of the spinnaker SDK examples. Program allows user to for testing of trigger
delay between trigger and beginnig of acquisition for Spinnaker Blackfly Camera S model
cameras by use of the Spinnaker SDK API to software trigger the cameras and GPIO output
from the cameras

See function comments for more details.*/

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <wiringPi.h>
#include <stdio.h>
#include <fstream>
#include <time.h>
#include <string>
#include "include/features.cpp"
#include <future>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

std::ofstream myfile;

#define FILE_NAME "globalReset.csv"
#define CAM_NUM 4  // somehow pin 4 digitially maps to pin 23 on the pi

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
    setBool("V3_3Enable", pCam, nodeMap, nodeMapTLDevice, true);
return 0;
}
#endif

clock_t listenForGPIO() {
    while(digitalRead(CAM_NUM) == 0) {
	continue;
    }
    return clock();
}

/* Called by run camera function. Takes as input camera, camera nodeMap, and cameras
nodeMapTLDevice. Acquires a single image and saves this image in the specified directory
with the name Acquisition-(Date & time).jpg. Works under assumption that camera is in
Acquisition mode. */
int getImage(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, int t_exp) {
    std::future<clock_t> f = std::async(std::launch::async, listenForGPIO);
    ImagePtr pResultImage;
    clock_t trigger = clock();
    pResultImage = pCam->GetNextImage();
    clock_t exposure = f.get();
    myfile.open(FILE_NAME, std::ios::app);
    myfile << exposure;
    myfile << ",";
    myfile << trigger;
    myfile << ",";
    myfile << t_exp;
    myfile << "\n";
    myfile.close();
    if (pResultImage->IsIncomplete()) {
        cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << endl;
        return -1;
    } else {
        pResultImage->Release();
        return 0;
    }
}

/* Function called by main in case where only one camera connected. Function calls PrintDeviceInfo
and setAcquisitionMode, setting up the camera and generates cli allowing user to take photos via
function getImages, change exposure time via function setExposureTime, change adc bit depth via function setADCBitDepth, and change shutter mode via function setShutterMode */
int runSingleCamera(CameraPtr pCam, SystemPtr system, CameraList camList) {
    wiringPiSetup();
    pinMode(CAM_NUM, INPUT);
    //myfile.open(FILE_NAME);
    //myfile << "GPIO Output,Trigger, Exposure Time, clock ticks per second=";
    //myfile << CLOCKS_PER_SEC;
    //myfile << "\n";
    //myfile.close();
    INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
    PrintDeviceInfo(nodeMapTLDevice);

#ifdef _DEBUG
        cout << endl << endl << "*** DEBUG ***" << endl << endl;

        // If using a GEV camera and debugging, should disable heartbeat first to prevent further issues
        if (DisableHeartbeat(nodeMap, nodeMapTLDevice) != 0)
        {
            return -1;
        }

#endif

    //vector<int> exposureTimeList = get_exposure_times("RollingShutter");
    vector<int> exposureTimeList = {400, 400, 400}; 
    for (int t_exp : exposureTimeList) {
	pCam->Init();
   	INodeMap& nodeMap = pCam->GetNodeMap();
   	set("AcquisitionMode", pCam, nodeMap, nodeMapTLDevice, "SingleFrame");
  	setPixelFormat(pCam, nodeMap, nodeMapTLDevice, 16);
   	set("LineSelector", pCam, nodeMap, nodeMapTLDevice, "Line2");
   	set("LineMode", pCam, nodeMap, nodeMapTLDevice, "Output");
  	set("LineSource", pCam, nodeMap, nodeMapTLDevice, "exposureActive");
	setShutterMode(pCam, nodeMap, nodeMapTLDevice, 1);
	setExposureTime(pCam, nodeMap, nodeMapTLDevice, t_exp);
	for(int i = 0; i < 10; i++) {
	   pCam->BeginAcquisition();
	   cout << "getting next image" << endl;
	   getImage(pCam, nodeMap, nodeMapTLDevice, t_exp);
	   pCam->EndAcquisition();
	}
	pCam->DeInit();
    }
    return 0;
}

/* Main function takes as input no arguments. Determines number of cameras and calls corresponding run
camera(s) function. */
int main(int /*argc*/, char** /*argv*/) {
    // Print application build information
    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;

    // Retrieve singleton reference to system object
    SystemPtr system = System::GetInstance();

    // Print out current library version
    const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();
    cout << "Spinnaker library version: " << spinnakerLibraryVersion.major << "." << spinnakerLibraryVersion.minor
         << "." << spinnakerLibraryVersion.type << "." << spinnakerLibraryVersion.build << endl
         << endl;

    CameraList camList = system->GetCameras();

    const unsigned int numCameras = camList.GetSize();

    cout << "Number of cameras detected: " << numCameras << endl << endl;

    // Finish if there are no cameras
    if (numCameras != 1) {
        // Clear camera list before releasing system
        camList.Clear();

        // Release system
        system->ReleaseInstance();
        cout << "Not enough or too many cameras! Press Enter to exit." << endl;
        getchar();
        return -1;
    } 

    CameraPtr pCam = camList.GetByIndex(0);
    runSingleCamera(pCam, system, camList);
    pCam = nullptr;
    camList.Clear();
    system->ReleaseInstance();
    return 0;
}




















