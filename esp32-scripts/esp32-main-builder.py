__author__      = "Niklas Gaudlitz  & Marcel Seerig"
__copyright__   = "Copyright 2020, Electronic Design Chemnitz GmbH"
__version__     = "1.0"


import fileinput
import sys
import os
import subprocess
from pathlib import Path
import shutil

from include import partitiontable_lib
from include import mkspiffs_lib
from include import flash_lib
from include import definitions_hpp_lib
from include import sdkconfig_lib
from include import buildUpdate_lib

# global varriables
COMMAND = None
METHOD  = None
#config  = {}

"""
    Load arguments into config if given
"""
def loadArguments(config):

    paramlist = sys.argv[1].split(" ")
    if len(paramlist) > 1:
        #arguments ar now in paramlist
        config["platform"] = "Windows"
    else:
        #parsing for windows didn't work, try again
        config["platform"] = "Linux"
        paramlist = sys.argv # copy args to a non system list
        paramlist.pop(0)

    COMMAND = paramlist[0]
    METHOD  = paramlist[1]

    # iterrate over parameter "key=value" pairs and replace key values form project config with arguments, if given

    if len(paramlist) >= 3:
        for entry in paramlist:
            try:
                key, value = entry.split("=")
                config[key] = value
            except:
                #skip other
                pass

    return COMMAND, METHOD, config

"""
    Read csv like config form textfile.
"""
def readConfig(file):
    config = {}
    for line in fileinput.input(file):
        if (line[0] != '#') and (line[0] != '\n'):
            (Name, Path) = line.rstrip().split(",")
            config[Name] = Path.replace(" ", "")
    return config

"""
    Call idf.py with parameters
"""
def callIDF(param):
    sys.stdout.flush()
    cmd = f"idf.py {param}"
    print(cmd)
    ret = os.system(cmd)
    sys.stdout.flush()
    if(ret != 0):
        print("FAILED!")
        exit(-2)

"""
    Configure Build target
"""
def setTarget(target, chip):
    output = ""

    if (target == "debug"):
        output = "message(\"\\n\\n#### Build Projekt in DEBUG Configuration ####\\n\\n\" )\n"
        output+= "set(SDKCONFIG_DEFAULTS  sdkconfig_debug.defaults)"

    if(target == "release"):
        output = "message(\"\\n\\n#### Build Projekt in RELEASE Configuration ####\\n\\n\" )\n"
        output+= "set(SDKCONFIG_DEFAULTS  sdkconfig_release.defaults)"

    f = open("target.cmake", "w")
    f.write(output)
    print(output)
    f.close()

    callIDF("set-target "+chip)

"""
Do Renaming for clear file names
"""
def renameDebugFile():
    try:
        os.remove("build/app_debug.bin")
    except:
        print("no build/app_debug.bin to remove..")
    shutil.copyfile("build/app.bin", "build/app_debug.bin")

def main():

    print("\n\n"+str(sys.argv))

    # Step0 - Reading file locations from projectConfig.csv
    print("Step1 - Reading file locations from projectConfig.csv")
    config = readConfig(Path.cwd() / 'projectConfig.csv')

    # STEP1 - Parsing commands from caller script
    COMMAND, METHOD, config = loadArguments(config)
    print("RUN "+str(COMMAND)+" "+str(METHOD)+" "+str(config["target"]+" "+config["chip"]))

    # Step2 - Init Modules
    sdkconfig = sdkconfig_lib.Sdkconfig(config)
    partitiontable = partitiontable_lib.Partitiontable(config, sdkconfig)
    mkspiffs = mkspiffs_lib.MKSPIFFS(config, partitiontable)
    flash = flash_lib.Flash(config, partitiontable)
    definitions = definitions_hpp_lib.Definitionshpp(config)
    update = buildUpdate_lib.Update(config, definitions)

    # SYNTAX
    # python esp32-main-builder.py <COMMAND> <METHOD> --<taget>

    # BUILD
    # --- app
    # --- data
    # --- update
    # --- clean
    # --- fullclean
    # FLASH
    # --- app
    # --- data
    # --- update
    # --- erase
    # PREPARE
    # --- menuconfig
    # --- encrypt-chip
    # READ
    # --- <partitionName>
    # DEBUG
    # --- monitor

    # Command Handling ########################################################################################
    print(config)
    if(COMMAND == "build"):
        setTarget(config["target"], config["chip"])
        try:
            os.remove("sdkconfig")
        except:
            print("no sdkconfig to remove..")

        if(METHOD == "app"):

            if(config["target"] == "debug"):
                callIDF("build")
                renameDebugFile()

            if(config["target"] == "release"):
                definitions.transformToRelease()
                callIDF("bootloader")
                callIDF("build")
                flash.encrypt("bootloader", "build/bootloader/bootloader.bin", "build/bootloader/bootloader_en.bin")
                flash.encrypt("partition-table", "build/partition_table/partition-table.bin", "build/partition_table/partition-table_en.bin")
                flash.encrypt("app_0", "build/app.bin", "build/app_release.bin")

        if(METHOD == "clean"):
            callIDF("clean")

        if(METHOD == "fullclean"):
            callIDF("fullclean")

        if(METHOD == "data"):
            mkspiffs.build()

        if(METHOD == "update"):

            if(config["target"] == "debug"):
                callIDF("build")
                mkspiffs.build()
                update.build()
                renameDebugFile()

            if(config["target"] == "release"):
                callIDF("bootloader")
                callIDF("build")
                flash.encrypt("bootloader", "build/bootloader/bootloader.bin", "build/bootloader/bootloader_en.bin")
                flash.encrypt("partition-table", "build/partition_table/partition-table.bin", "build/partition_table/partition-table_en.bin")
                flash.encrypt("app_0", "build/app.bin", "build/app_release.bin")
                mkspiffs.build()
                update.build()

    if(COMMAND == "flash"):

        if(METHOD == "app"):

            if(config["target"] == "debug"):
                flash_list = [
                    ["bootloader",      "build/bootloader/bootloader.bin"],
                    ["partition-table", "build/partition_table/partition-table.bin"],
                    ["otadata",         "build/ota_data_initial.bin"],
                    ["phy_init",        "build/phy_init_data.bin"],
                    ["app_0",           "build/app_debug.bin"]
                ]
                flash.write(flash_list)

            if(config["target"] == "release"):
                flash_list = [
                    ["bootloader",      "build/bootloader/bootloader_en.bin"],
                    ["partition-table", "build/partition_table/partition-table_en.bin"],
                    ["otadata",         "build/ota_data_initial.bin"],
                    ["phy_init",        "build/phy_init_data.bin"],
                    ["app_0",           "build/app_release.bin"]
                ]
                flash.write(flash_list)

        if(METHOD == "data"):
            flash.write([[ "data_0", "build/data.bin" ]])

        if(METHOD == "update"):

            if(config["target"] == "debug"):
                flash_list = [
                    ["bootloader",      "build/bootloader/bootloader.bin"],
                    ["partition-table", "build/partition_table/partition-table.bin"],
                    ["otadata",         "build/ota_data_initial.bin"],
                    ["phy_init",        "build/phy_init_data.bin"],
                    ["app_0",           "build/app_debug.bin"],
                    ["data_0",          "build/data.bin"]
                ]
                flash.write(flash_list)

            if(config["target"] == "release"):
                flash_list = [
                    ["bootloader",      "build/bootloader/bootloader_en.bin"],
                    ["partition-table", "build/partition_table/partition-table_en.bin"],
                    ["otadata",         "build/ota_data_initial.bin"],
                    ["phy_init",        "build/phy_init_data.bin"],
                    ["app_0",           "build/app_release.bin"],
                    ["data_0",          "build/data.bin"]
                ]
                flash.write(flash_list)

        if(METHOD == "erase"):
            flash.erase()


    if(COMMAND == "prepare"):

        if(METHOD == "menuconfig"):
            setTarget(config["target"], config["chip"])
            subprocess.call("start /wait idf.py menuconfig", shell=True)

        if(METHOD == "encrypt-chip"):

            flash.flash_encryption_preparation()



    if(COMMAND == "read"):

        flash.read(METHOD)


    if(COMMAND == "debug"):
        setTarget(config["target"], config["chip"])

        if(METHOD == "monitor"):
            subprocess.call("start /wait idf.py -p "+config["flash_port"]+" monitor", shell=True)

        if(METHOD == "openOCD"):
            subprocess.call("start openocd -f esp32-scripts/esp32-smartfit.cfg", shell=True)

    print("\nDONE\n")
    sys.exit(0)


if __name__ == "__main__":
    main()