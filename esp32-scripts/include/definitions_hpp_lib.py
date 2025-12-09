__author__      = "Niklas Gaudlitz & Marcel Seerig"
__copyright__   = "Copyright 2020, Electronic Design Chemnitz GmbH"
__version__     = "1.0"

import fileinput

class Definitionshpp:
    def __init__(self, config):
        self.config = config
        try:
            for line in fileinput.input(self.config["definitionshpp"]):
                if(line.find("#define MODULE_TYPE") != -1):
                    start = line.find("\"")
                    end = line.find("\"", start+1)
                    if (start>0 and end >0):
                        self._Module_Type = line[start+1:end]
        except:
            self._Module_Type = ""
        
        try:
            for line in fileinput.input(self.config["definitionshpp"]):
                if(line.find("#define FIRMWARE_VERSION") != -1):
                    start = line.find("\"")
                    end = line.find("\"", start+1)
                    if (start>0 and end >0):
                        self._Firmware_Version = line[start+1:end]
        except:
            self._Module_Type = ""
    
    def getModuleType(self):
        return self._Module_Type
    
    def getFirmwareVersion(self):
        return self._Firmware_Version
    
    def transformToRelease(self):
        # delete "#define DEBUG_SITUATION"
        # delete " PROTOTYPE" from strings
        try:
            for line in fileinput.input(self.config["definitionshpp"], inplace=True):
                # if line contains 'DEBUG_SITUATION'
                if line.find("#define DEBUG_SITUATION") != -1:
                    # if this is not comment out
                    if line.find("//#define DEBUG_SITUATION") != -1:
                        # comment line out
                        line = line.replace("#define DEBUG_SITUATION", "//#define DEBUG_SITUATION")
                print(line, end='')
            print("Transformed Definitions.hpp")
        except:
            print("Transforming Definitions.hpp failed")