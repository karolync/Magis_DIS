/* File written by Jonah Ezekiel (jezekiel@stanford.edu). Program allows for software interfacing with
 * raspberry pi gpio pins. Intended to be used as a dependancy for files a directory up, allowing those
 * files to use GPIO to trigger relay switches and hence power cycle cameras.
 *		
 * See function comments for more details. */

#include <iostream>
extern "C" {
    #include <wiringPi.h>
}
#include <stdio.h>
#include <stdlib.h>

#define out 23

using namespace std;

void closeRelay() {
    cout << "Closing relay" << endl;
    digitalWrite(out, true);
}

void openRelay() {
    cout << "Opening relay" << endl;
    digitalWrite(out, false);
}

void setupRelay() {
    wiringPiSetup();
    pinMode(out, OUTPUT);
}













