/* File written by Jonah Ezekiel (jezekiel@stanford.edu). Program allows for software interfacing with
 * raspberry pi gpio pins. Intended to be used as a dependancy for files a directory up, allowing those
 * files to use GPIO to trigger relay switches and hence power cycle cameras.
 *		
 * See function comments for more details. */

#include <iostream>
#include "<lgpio.h>"
#include <stdio.h>
#include <stdlib.h>

#define out 4

using namespace std;

int h;
int lFlags;

void closeRelay() {
    cout << "Closing relay" << endl;
    lgGpioWrite(h, out, 1);
}

void openRelay() {
    lgGpioWrite(h, out, 0);
}

void setupRelay() {
    lFlags = 0;
    h = lgGpiochipOpen(0);
    lgGpioClaimOutput(h, lFlags, out, 0);
}













