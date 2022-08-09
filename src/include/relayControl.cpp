/* File written by Jonah Ezekiel (jezekiel@stanford.edu). Program allows for software interfacing with
 * raspberry pi gpio pins. Intended to be used as a dependancy for files a directory up, allowing those
 * files to use GPIO to trigger relay switches and hence power cycle cameras.
 *		
 * See function comments for more details. */

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

#define RELAY_NUM 5 //Somehow pin 5 digitially maps to pin 24 on the pi

using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

void closeRelay() {
    cout << "Closing relay" << endl;
    digitalWrite(RELAY_NUM, true);
}

void openRelay() {
    cout << "Opening relay" << endl;
    digitalWrite(RELAY_NUM, false);
}

void inputFunction() {
    cout << "input recieved" << endl;
}

void setupGPIO(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice) {
    cout << "Setting up relay on gpio pin " << RELAY_NUM << endl;
    wiringPiSetup();
    pinMode(RELAY_NUM, OUTPUT);
    set("LineSelector", pCam, nodeMap, nodeMapTLDevice, "Line2");
    set("LineMode", pCam, nodeMap, nodeMapTLDevice, "Output");
    setBool("V3_3Enable", pCam, nodeMap, nodeMapTLDevice, true);
    set("LineSource", pCam, nodeMap, nodeMapTLDevice, "ExposureActive");
}











