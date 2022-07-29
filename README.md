# Magis-100 DIS Github Repository
This repository was developed by Jonah Ezekiel (jezekiel@stanford.edu) on behalf of and with guidance from the SLAC Magis team to aid in the development and planning for the diagnostic imaging system for the Magis-100 experiment. This repository provides the functionality to both trigger and modify feature variables of Flir Blackfly Model S cameras over software with the purpose of developing a system to easily characterize these cameras and test hardware the layout planned for interacting with these cameras inside the Magis-100 experiment, shown below: 

   Ethernet
      |
DC -- PI --(usb 3)-- Pi-controlled Relay --(usb 3)--\        /--(usb 3)-- Cam 3 (Physics)
                                                     USB HUB --(usb 3)-- Cam 2
DC -- Pi-controlled Relay --------------------------/        \--(usb 3)-- Cam 1
