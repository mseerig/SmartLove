__author__      = "Niklas Gaudlitz & Marcel Seerig"
__copyright__   = "Copyright 2020, Electronic Design Chemnitz GmbH"
__version__     = "1.0"

import fileinput
import sys

class Partitiontable:

    def __init__(self, config, sdkconfig):
        self.config = config
        self._Names = []
        self._Type = {}
        self._SubType = {}
        self._Offset = {}
        self._Size = {}
        self._Flags = {}

        # Load Partitions csv

        f_name = "partitions_"+ self.config["target"] +".csv"

        for line in fileinput.input(f_name):
          if (line[0] != '#') and (line[0] != '\n'):
             try:
                (Name, Type, SubType, Offset, Size, Flags) = line.rstrip().split(',')
                self._Names.append(Name)
                self._Type[Name] = Type.replace(" ", "")
                self._SubType[Name] = SubType.replace(" ", "")
                self._Offset[Name] = Offset.replace(" ", "")
                self._Size[Name] = Size.replace(" ", "")
                self._Flags[Name] = Flags.replace(" ", "")
             except:
                (Name, Type, SubType, Offset, Size) = line.rstrip().split(',')
                self._Names.append(Name)
                self._Type[Name] = Type.replace(" ", "")
                self._SubType[Name] = SubType.replace(" ", "")
                self._Offset[Name] = Offset.replace(" ", "")
                self._Size[Name] = Size.replace(" ", "")

        # Load content from sdkconfig
        self._Names.append("bootloader")
        self._Type["bootloader"] = "bootloader"
        self._SubType["bootloader"] = ""
        self._Offset["bootloader"] = sdkconfig.getBootloaderOffset()
        self._Size["bootloader"] = ""

        self._Names.append("partition-table")
        self._Type["partition-table"] = "partition-table"
        self._SubType["partition-table"] = ""
        self._Offset["partition-table"] = sdkconfig.getPartitiontableOffset()
        self._Size["partition-table"] = ""


    def getType(self, Name):
        return self._Type[Name]

    def getSubType(self, Name):
        return self._SubType[Name]

    def getOffset(self, Name):
        return self._Offset[Name]

    def getSize(self, Name):
        return self._Size[Name]

    def getFlag(self, Name):
        return self._Flags[Name]


###################################################


    def getNames(self):
        return self._Names

    def getTypes(self):
        return self._Type
    
    def getSubTypes(self):
        return self._SubType

    def getOffsets(self):
        return self._Offset

    def getSizes(self):
        return self._Size

    def getFlags(self):
        return self._Flags

    
#########################################