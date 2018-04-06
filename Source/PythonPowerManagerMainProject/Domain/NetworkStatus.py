#!/usr/bin/python

import timeit;
import logging;

class NetworkStatus(object):
    def __init__(self, executionInterval, errorLimit):
        self.errorCount = 0;
        self.errorLimit = errorLimit;
        self.executionInterval = executionInterval;
        self.lastExecutionTime = timeit.default_timer();
    def operationStatus(self, status):
        logging.debug(status);
        self.lastExecutionTime = timeit.default_timer();
        if status != 'alive':
            self.errorCount = self.errorCount + 1;
        else:
            self.errorCount = 0;
    def isExecuteMonitoring(self):
        if timeit.default_timer() - self.lastExecutionTime > self.executionInterval:
            return True;
        else:
            return False;