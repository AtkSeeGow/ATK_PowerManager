#!/usr/bin/python

import logging
import threading
import subprocess

from threading import Timer
from datetime import datetime

class NetworkService(object):
    def __init__(self, serialService, config):
        self.serialService = serialService;
        self.config = config;
    def executeMonitoring(self, internetStatus, websideStatus, domainStatus):
        self.operationNetworkMonitoring(internetStatus, websideStatus, domainStatus);
        self.operationPowerMonitoring(internetStatus, websideStatus, domainStatus);
        timer = Timer(1, self.executeMonitoring, (internetStatus, websideStatus, domainStatus))
        timer.start()
    def executeCommand(self, command):
        logging.debug(command);
        p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        output = p.communicate()[0].split('\n')[0]
        return output
    def operationNetworkMonitoring(self, internetStatus, websideStatus, domainStatus):
        if internetStatus.isExecuteMonitoring():
            internetStatus.operationStatus(self.executeCommand(self.config.commandInternetStatus));

        if websideStatus.isExecuteMonitoring():
            websideStatus.operationStatus(self.executeCommand(self.config.commandWebsideStatus));

        if domainStatus.isExecuteMonitoring():
            domainStatus.operationStatus(self.executeCommand(self.config.commandDomainStatus));

        self.serialService.write("L#%s#I:%02d W:%02d D:%02d"%(datetime.now().strftime("%Y%m%d %H%M%S"), internetStatus.errorCount, websideStatus.errorCount, domainStatus.errorCount))
    def operationPowerMonitoring(self, internetStatus, websideStatus, domainStatus):
        if internetStatus.errorCount > internetStatus.errorLimit or websideStatus.errorCount > websideStatus.errorLimit or domainStatus.errorCount > domainStatus.errorLimit:
            logging.debug("NetworkService operationPowerMonitoring power off");
            internetStatus.errorCount = 0;
            websideStatus.errorCount = 0;
            domainStatus.errorCount = 0;
            self.serialService.write("W#%s" % (180 * 1000))

