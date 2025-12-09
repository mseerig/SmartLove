__author__      = "Niklas Gaudlitz & Marcel Seerig"
__copyright__   = "Copyright 2020, Electronic Design Chemnitz GmbH"
__version__     = "1.0"

import fileinput
import sys

class Sdkconfig:

    def __init__(self, config):
        self.config = config
        self.bootloaderOffset = None
        self.partitiontableOffset = None

        for line in fileinput.input("sdkconfig_"+config["target"]+".defaults"):
            if (line[0] != '#') and (line[0] != '\n'):
                try:
                    (Name, Value) = line.split('=')
                    Value = Value.replace("\n","")

                    if(Name == 'CONFIG_BOOTLOADER_OFFSET_IN_FLASH'):
                        self.bootloaderOffset = Value

                    if(Name == 'CONFIG_PARTITION_TABLE_OFFSET'):
                        self.partitiontableOffset = Value

                except:
                    pass

        if (self.bootloaderOffset == None):
            print("Set BOOTLOADER_OFFSET_IN_FLASH to 0x1000 default")
            self.bootloaderOffset = "0x1000"

        if (self.partitiontableOffset == None):
            print("CONFIG_PARTITION_TABLE_OFFSET not found!")
            print("Error!")
            exit()

    def getBootloaderOffset(self):
        return self.bootloaderOffset

    def getPartitiontableOffset(self):
        return self.partitiontableOffset