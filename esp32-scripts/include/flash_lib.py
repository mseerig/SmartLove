__author__      = "Marcel Seerig"
__copyright__   = "Copyright 2020, Electronic Design Chemnitz GmbH"
__version__     = "1.0"

import os
import sys

flash_key_bin = "build/flash_encryption_key.bin"

class Flash:
    def __init__(self, config, partitiontable):
        self.config = config
        self.partition = partitiontable

    def read(self, partition_name):
        if (partition_name in self.partition.getNames()):
            cmd =  "python " + self.config['esp_tool_dir'] + "/esptool.py"
            cmd += " --chip "+self.config['chip']+" --port " + self.config['flash_port'] + " --baud " + self.config['baudrate']
            cmd += " read_flash " + self.partition.getOffset(partition_name)
            cmd += " " + self.partition.getSize(partition_name)
            cmd += " read_" + partition_name + ".bin"
            print(cmd)
            sys.stdout.flush()
            os.system(cmd)
        else:
            print("## '" + partition_name + "' was not in partition table!")
            sys.stdout.flush()


    def write(self, flist):
        flash_list = ""
        i = 0 
        # add partitions to flash list
        for entry in flist:

            if(entry[0] in self.partition.getNames()):
                flash_list += " " + self.partition.getOffset(entry[0]) 
                flash_list += " " + entry[1]
            i+=1
           
        if(i == 0):
            print("INFO: nothing to flash")
            return

        # flash to chip
        cmd =  "python " + self.config['esp_tool_dir'] + "/esptool.py"
        cmd += " --chip "+self.config['chip']+" --port " + self.config['flash_port'] + " --baud " + self.config['baudrate']
        cmd += " --before default_reset --after hard_reset write_flash -z"
        cmd += " --flash_mode dio --flash_freq 80m --flash_size detect --force"
        cmd += flash_list
        print(cmd)
        sys.stdout.flush()
        os.system(cmd)


    # erase entire flash
    def erase(self):
        cmd =  "python " + self.config['esp_tool_dir'] + "/esptool.py"
        cmd += " --chip "+self.config['chip']+" --port " + self.config['flash_port'] + " --baud " + self.config['baudrate']
        cmd += " erase_flash"
        print(cmd)
        sys.stdout.flush()
        os.system(cmd)

    def encrypt(self, partition_name, source, destinaton):
        # encrypt partition
        cmd =  "python " + self.config['esp_tool_dir'] + "/espsecure.py"
        cmd += " encrypt_flash_data --aes_xts --keyfile " + self.config['flash_encryption_key']
        cmd += " --address " + self.partition.getOffset(partition_name) +" -o"
        cmd += " " + destinaton
        cmd += " " + source

        print(cmd)
        sys.stdout.flush()
        os.system(cmd)


    # flash the generated key file to the chip
    def flash_encryption_preparation(self):

        fuses = [
            "SPI_BOOT_CRYPT_CNT 1",
            "DIS_DOWNLOAD_MANUAL_ENCRYPT"
        ]

        # burn fuses
        for fuse in fuses:
            cmd =  "python " + self.config['esp_tool_dir'] + "/espefuse.py --do-not-confirm"
            cmd += " --port " + self.config['flash_port'] + " burn_efuse " + fuse
            print(cmd)
            sys.stdout.flush()
            os.system(cmd)

        # protect fuses
        for fuse in fuses:
            cmd =  "python " + self.config['esp_tool_dir'] + "/espefuse.py --do-not-confirm"
            cmd += " --port " + self.config['flash_port'] + " write_protect_efuse " + fuse.split(" ")[0]
            print(cmd)
            sys.stdout.flush()
            os.system(cmd)

        # flash "flash_encryption_key"
        cmd =  "python " + self.config['esp_tool_dir'] + "/espefuse.py --do-not-confirm"
        cmd += " --port " + self.config['flash_port'] + " burn_key BLOCK_KEY1 " + self.config['flash_encryption_key']
        cmd += " XTS_AES_128_KEY"
        print(cmd)
        sys.stdout.flush()
        os.system(cmd)

        # flash signing key
        cmd =  "python " + self.config['esp_tool_dir'] + "/espefuse.py --do-not-confirm"
        cmd += " --port " + self.config['flash_port'] + " burn_key BLOCK_KEY0 "+ self.config['signing_key']+" SECURE_BOOT_DIGEST0"
        print(cmd)
        sys.stdout.flush()
        os.system(cmd)
