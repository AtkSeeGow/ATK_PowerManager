#!/usr/bin/python

import time
import logging

from datetime import datetime

from Domain.Config import Config
from Domain.NetworkStatus import NetworkStatus

from Service.LogginService import LogginService
from Service.NetworkService import NetworkService
from Service.SerialService import SerialService

if __name__ == '__main__':
    config = Config();

    serialService = SerialService(config.serialDevice, 9600)

    logginService = LogginService(config.logPath, config.logName);
    logging.debug('NetworkMonitoring initialization start.....................')

    networkService = NetworkService(serialService, config)

    internetStatus = NetworkStatus(300, 4);
    websideStatus = NetworkStatus(600, 3);
    domainStatus = NetworkStatus(900, 2);

    networkService.executeMonitoring(internetStatus, websideStatus, domainStatus);

    logging.debug('NetworkMonitoring initialization end.....................')