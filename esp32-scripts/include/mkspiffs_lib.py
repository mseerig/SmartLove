__author__      = "Niklas Gaudlitz & Marcel Seerig"
__copyright__   = "Copyright 2020, Electronic Design Chemnitz GmbH"
__version__     = "1.0"

import os
import sys
from pathlib import Path

class MKSPIFFS:
    # path to configfile, win or lin, partition instance
    def __init__(self, config, partitiontable):
        self.config = config
        self.partition = partitiontable

    def build(self):

        cmd =  "python "+self.config["esp_idf_path"]+"/components/spiffs/spiffsgen.py --obj-name-len 60 "
        cmd += self.partition.getSize("data_0") + " " + self.config["webfrontend"] + " build/data.bin"
        print(cmd)
        sys.stdout.flush()
        os.system(cmd)

        """
        if(self.config["platform"] == "Linux"):
            try:
                print("# RUN mkspiffs")
                sys.stdout.flush()

                cmd =  "esp32-scripts/include/mkspiffs -c " + self.config["webfrontend"] 
                cmd += " -b 4096 -p 256 -s " + self.partition.getSize("data_0") + " " + "data.bin"
                print(cmd)
                sys.stdout.flush()
                os.system(cmd)
            except:
                print("RUN mkspiffs failed for some reason!")

        if(self.config["platform"] == "Windows"):
            try:
                print("# RUN mkspiffs")
                sys.stdout.flush()

                cmd =  "esp32-scripts\\include\\mkspiffs.exe -c " + self.config["webfrontend"]
                cmd += " -b 4096 -p 256 -s " + self.partition.getSize("data_0") + " " + "build/data.bin"
                print(cmd)
                sys.stdout.flush()
                os.system(cmd)
            except:
                print("RUN mkspiffs.exe failed for some reason!")
        """