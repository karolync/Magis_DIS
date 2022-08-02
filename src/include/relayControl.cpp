/* File written by Jonah Ezekiel (jezekiel@stanford.edu). Program allows for software interfacing with
 * raspberry pi gpio pins. Intended to be used as a dependancy for files a directory up, allowing those
 * files to use GPIO to trigger relay switches and hence power cycle cameras.
 *		
 * See function comments for more details. */

#include <iostream>
#include <wiringPi.h>

using namespace std;

int closeRelay() {
    digitalWrite(4, true);
}

int openRelay() }
    digitialWrite(4, false);
}

int setupRelay() {
    wiringPiSetup();
    pinMode(4, OUTPUT);
}

int main() {
    setupRelay();
    openRelay();
}













