#!/usr/bin/python

import os
import logging

from logging.handlers import TimedRotatingFileHandler

class LogginService(object):
    def __init__(self, logPath, logName):
        formatter = logging.Formatter("%(asctime)s %(levelname)s - %(message)s")

        handler = TimedRotatingFileHandler(os.path.join(logPath, logName),when="MIDNIGHT",interval=1,backupCount=5)
        handler.setLevel(logging.DEBUG)
        handler.setFormatter(formatter)

        logger = logging.getLogger()
        logger.setLevel(logging.DEBUG)
        logger.addHandler(handler)