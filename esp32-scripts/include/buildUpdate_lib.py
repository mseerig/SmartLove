__author__      = "Niklas Gaudlitz & Marcel Seerig"
__copyright__   = "Copyright 2020, Electronic Design Chemnitz GmbH"
__version__     = "1.0"

import hashlib
import sys

class Update:

    # instance of definitions class, pathslist of main
    def __init__(self, config, definitions):
        self.config = config
        self.definitions = definitions

    def getSize(self,fileobject):
        fileobject.seek(0,2) # move the cursor to the end of the file
        size = fileobject.tell()
        fileobject.seek(0,0) # move back
        return size

    def generateSHA256Hash(self,fileobject):
    	b = fileobject.read() # read entire file as bytes
    	readable_hash = hashlib.sha256(b).hexdigest()
    	fileobject.seek(0,0) # move back
    	return readable_hash

    def build(self):

        try:
            # read firmwaretype from Definitions.hpp


            print("Firmware Type:     "+ self.definitions.getModuleType())
            print("App  Partition:    build/app.bin")
            print("Data Partition:    build/data.bin")
            print("New Update File:   build/update.bin")

            #open the app partition file
            app_file = open("build/app.bin", "rb")

            app_size = self.getSize(app_file)

            #open the data partition file
            data_file = open("build/data.bin", "rb")

            data_size = self.getSize(data_file)
            sha256Hash= self.generateSHA256Hash(data_file)

            #creating an new output file
            update_file = open("build/update.bin", "wb")


            # Wirte Header
            print("\nCreate Header...")
            print("TYPE:<"+ self.definitions.getModuleType() +">")
            print("APP:<"+str(app_size)+">")
            print("DATA:<"+str(data_size)+">")
            print("<"+str(sha256Hash)+">")
            print("START:\n")
            update_file.write(b"TYPE:<"+ self.definitions.getModuleType().encode('utf-8')+b">\n")
            update_file.write(b"APP:<"+str(app_size).encode('utf-8')+b">\n")
            update_file.write(b"DATA:<"+str(data_size).encode('utf-8')+b">")
            update_file.write(b"<"+str(sha256Hash).encode('utf-8')+b">\n")
            update_file.write(b"START:\n")
            update_file.flush()

            #now wirte to update file
            print("Add App-Partition...")
            update_file.write(app_file.read())
            app_file.close()

            print("Add Data-Partition...")
            update_file.write(data_file.read())
            data_file.close()

            update_file.close()
            print("\n-> New Update File was created with success!")
            sys.exit(0)
        except SystemExit:
	        print("leave!")
            
        except:
            print("\n-> ERROR!")
            sys.exit(1)