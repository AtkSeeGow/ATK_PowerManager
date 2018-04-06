#!/usr/bin/python

class Config(object):
    def __init__(self):
        self.serialDevice = "/dev/ttyUSB0"

        self.logPath = "/home/pi/Log"
        self.logName = "NetworkMonitoring"

        self.commandInternetStatus = "fping 8.8.8.8 | awk '{print $3 }'"
        self.commandWebsideStatus = "fping 192.168.0.1 | awk '{print $3 }'"
        self.commandDomainStatus = "fping 8F3B62AF-2063-4508-9A35-9C0C42623943 | awk '{print $3 }'"