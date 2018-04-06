#!/usr/bin/python

import serial

class SerialService(object):
    def __init__(self, devicePath, baudRate):
        self.serial = serial.Serial(devicePath, baudRate)
    def write(self, command):
        self.serial.write(command + '\n')
    def read(self):
        return self.serial.readline()
