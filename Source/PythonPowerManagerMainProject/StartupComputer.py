#!/usr/bin/python

from Domain.Config import Config
from Service.SerialService import SerialService

if __name__ == '__main__':
    config = Config()
    
    serialService = SerialService(config.serialDevice, 9600)
    serialService.write('S#100\n')
    
    exit()