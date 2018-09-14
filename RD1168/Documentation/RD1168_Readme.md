# Introduction:

This reference design is for the MXCHIP EMW3166 module and uses the MTB / MCB HW for implementation. The example uses "mbed-cloud-client" library that handles all the connection, registration and LWM2M resources. The example simulates a button press every 5 seconds and relays the packet to Arm Pelion IoT platform.

The reference application also currently works on Mbed OS v5.9.5 and Mbed Cloud Client v1.3.3.

# Pre-requisites:
1. Hardware:
    * MXCHIP EMW3166 MTB / MCB with SPI flash + micro USB cable.
    * A WiFi access point.

2. Software:
    * mbed CLI - https://github.com/ARMmbed/mbed-cli
    * manifest tool - ``` pip install git+https://github.com/ARMmbed/manifest-tool --upgrade ```
    * Both the above tools will be combined into mbed CLI (via command ``` dm ``` in a later release of mbed CLI)
    * Pelion account for API keys & developer certificate.

# Getting started:
1. Import this example to your local machine with mbed CLI:
    ``` mbed import https://github.com/ARMmbed/reference-designs.git ```

2. Move into the RD1168 directory ``` cd RD1168 ```
    * Login to your Pelion Cloud account on a browser & follow the steps below:
        * Navigate to Device identity > Certificates
        * Select the certificate created by your account admin and click on "Download Developer C file"
        * Save the file mbed_cloud_dev_credentials.c to the location of the RD1168 directory. You have to overwrite the contents of the existing file there.

3. Configure your network credentials:
    * In the mbed_app.json file in the root of the application, modify the WiFi SSID and password for your network.
    * Save the changes.

4. The supplied Bootloader in ../tools/ is currently built to use SPI Flash. If you are using the SPI Flash to store the certificates, then you are good to go. If you prefer to use the SD card for the storage, follow the steps below. If not, you may skip these.
   
5. To build the bootloader to use the SD card: TIP: You may want to use atleast a Class 10, 2GB SD card.
    * Import the bootloader repo - https://github.com/ARMmbed/mbed-bootloader
    * Change main.cpp with:
    ```
        #if MBED_CLOUD_CLIENT_UPDATE_STORAGE == ARM_UCP_FLASHIAP_BLOCKDEVICE
        #include "SDBlockDevice.h"  
        /* initialise sd card blockdevice */  
        #if defined(MBED_CONF_APP_SPI_MOSI) && defined(MBED_CONF_APP_SPI_MISO) && \  
   defined(MBED_CONF_APP_SPI_CLK)  && defined(MBED_CONF_APP_SPI_CS)  
   SDBlockDevice sd(MBED_CONF_APP_SPI_MOSI, MBED_CONF_APP_SPI_MISO,  
   MBED_CONF_APP_SPI_CLK,  MBED_CONF_APP_SPI_CS);  
   ```

    * Modify mbed_app.json in bootloader: Change update storage-address and remove SPI pin defines.  
    
    ```
    "update-client.storage-address"  : "(1024*1024*64)",  
    "MTB_MXCHIP_EMW3166": {  
    "flash-start-address"              : "0x08000000",  
    "flash-size"                       : "(1024*1024)",  
    "sotp-section-1-address"           : "(MBED_CONF_APP_FLASH_START_ADDRESS+768*1024)",  
    "sotp-section-1-size"              : "(128*1024)",  
    "sotp-section-2-address"           : "(MBED_CONF_APP_FLASH_START_ADDRESS+896*1024)",  
    "sotp-section-2-size"              : "(128*1024)",  
    "update-client.application-details": "(MBED_CONF_APP_FLASH_START_ADDRESS+64*1024)",  
    "application-start-address"        : "(MBED_CONF_APP_FLASH_START_ADDRESS+65*1024)",  
    "max-application-size"             : "DEFAULT_MAX_APPLICATION_SIZE",   
    }  
   ```
6. Change mbed_app.json in source:
 ```   "update-client.storage-address"  : "(1024*1024*64)",  ```
   
7. Once changes are made in bootloader, build it:
``` mbed compile -t ARM -m MTB_MXCHIP_EMW3166 ```

8. Copy the generated binary to ../tools/ directory. Rename to match the existing file name.

9. Change mbed_app.json in the root of the reference application i.e. 
"update-client.storage-address"  : "(1024*1024*64)"
10. Change main.cpp in source to match SD card
### IMPORTANT: The required lines are already supplied in the code. Uncomment appropriate lines in main.cpp and build the binary.

11. Combine with the bootloader using the supplied script in ../tools/ .
``` $> python tools/combine_bootloader_with_app.py -m MTB_MXCHIP_EMW3166 -a BUILD/MTB_MXCHIP_EMW3166/ARM/mbed-cloud-example.bin -o combined.bin ```

12. Flash the combined binary (combined.bin) to your device. Open a terminal & observe the output.

13. The device starts the example, connects to the WiFi network & after a few moments connects & registers to Pelion Cloud. It then prints out a device ID. You will need this for firmware update at a later stage.

* You can now check your simulated button counter's value in the Pelion portal as well.
* Navigate to "Device Directory" in the portal and click on your device's device ID. This will open up a new drawer on the page.
* Scroll down to LWM2M resource "Button_count" and click on it.
* A new window opens and you will be able to see the counter & a graph increment every time the resource value increments.

14. Congratulations, your device is now connected to Pelion IoT platform. Please proceed to the FW update section now.

## Recommendation:  
Increase stack size:         "MBED_CONF_APP_MAIN_STACK_SIZE=5000", to 6K if using newer versions of Mbed OS  

#Demonstrate a remote Firmware update:

1. In order to update FW on a connected device, you will need the manifest tool. Please note that in a later release of Mbed OS & Mbed CLI, the manifest tool is integrated into the Mbed CLI (via the dm command) and you wouldn't need to manually install this.

Install the manifest tool via : ``` pip install git+https://github.com/ARMmbed/manifest-tool --upgrade ```

2. A dependency is to install the python SDK along with the manifest tool. This is done using : ``` pip install mbed-cloud-sdk --upgrade ```

3. Now you need to setup Mbed OS on the device to receive FW updates:
	* Login to your Pelion Cloud portal.
	* Create an API device key (Access Management -> API Keys -> Create a new API key)
	* Ensure you copy the API key generated as this is visible only once.
	
4. Initialize the certificates for your application:
##IMPORTANT: You must initialize the certificates before you flash the device for the 1st time as these certificates need to be embedded within the device in order for it to receive remote updates.

``` manifest-tool init -a <api key> -S <mbed cloud API URL> -d <domain name> -m <model ID> --force ```

Note the use of --force as this option overrides the defaults provided in the application. This is a mandatory parameter.

5. With the certificates initialized, there will now be a ".certificates" directory created within the application's root directory.

6. Now, compile the application:
``` mbed compile -t GCC_ARM -m MTB_MXCHIP_EMW3166 ```

7. Combine the application with the bootloader - use the provided one in the \tools\ directory if you are using the SPI flash. If you have built a new bootloader to use the SD card, then you need to use the new bootloader.

``` python tools/combine_bootloader_with_app.py -m MTB_MXCHIP_EMW3166 -a BUILD/MTB_MXCHIP_EMW3166/GCC_ARM/RD1168_source.bin -o combined.bin ```

8. Program your device with the ``` combined.bin ``` just generated. You can either drag-drop via the GUI of your OS or you can use the corresponding copy command from the terminal for your OS.

9. Open a serial terminal application such as PuTTY and observe the output. You will notice the device boots up, connects to the WiFi network, starts Pelion Cloud Client and registers to the Cloud.

10. Once the device registers, make a note of the device ID assigned to the device.

11. Modify the main.cpp in the source directory to add a small printf statement - this is already provided in the code. So, you can un-comment this line in order to demo a FW update.

12. Compile the new source with ``` mbed compile ``` command.

13. You will now have a new binary in the BUILD\MTB_MXCHIP_EMW3166\GCC_ARM\ directory.

14. Use the manifest-tool to perform a remote update on the device:

``` manifest-tool update device -p <payload name> -D <device id> ``` where payload name is the full path to your new binary built from step 13 and device ID is the one obtained from step 9.

15. You will notice from the serial terminal that the device gets a request to update the FW, then this is authorized, then the download of the new FW starts, finishes and the bootloader verifies the authencity of the new FW, installs it on the device and reboots.

16. You will now notice the new printf statement appear on the device logs which indicates that the newly built application binary is now installed and running on the device i.e. the device has been remotely updated.

The logs should look like this on a serial terminal:

..\Modules\Reference_Apps\RD1168\mbed-cloud-example>mbed sterm -b 115200 -r
[mbed] Detecting connected targets/boards to your system...
[mbed] Opening serial terminal to "MTB_MXCHIP_EMW3166"
--- Terminal on COM91 - 115200,8,N,1 ---
C
[BOOT] Active firmware integrity check:
[BOOT] [++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++]
[BOOT] SHA256: D82A3C281A3D4EB434D1ED9D1459166EA6A50C7BF8BC315694B6CC09C5E7DF0B
[BOOT] Version: 1534927885
[BOOT] Slot 0 is empty
[BOOT] Active firmware up-to-date
[BOOT] Application's start address: 0x8010400
[BOOT] Application's jump address: 0x80110C5
[BOOT] Application's stack address: 0x20040000
[BOOT] Forwarding to application...
��  � Starting Simple Mbed Cloud Client example
Connecting to the network using WiFi...
Init
Init status = 0
spif size: 4194304
spif read size: 1
spif erase size: 4096
Connected to the network successfully. IP address: 192.168.0.121
[Simple Cloud Client] Autoformatting the storage.
[Simple Cloud Client] Reset storage to an empty state.
[Simple Cloud Client] Starting developer flow
client_status = 0
Initialized Mbed Cloud Client. Registering...
Simulated button clicked 1 times
Simulated button clicked 2 times
Connected to Mbed Cloud. Endpoint Name: 016560d6b3cd0000000000010010025e
Simulated button clicked 3 times
Simulated button clicked 4 times
Simulated button clicked 5 times
Simulated button clicked 6 times
Simulated button clicked 7 times
Simulated button clicked 8 times
Simulated button clicked 9 times
Simulated button clicked 10 times
Simulated button clicked 11 times
Simulated button clicked 12 times
Simulated button clicked 13 times
Simulated button clicked 14 times
Simulated button clicked 15 times

Firmware download requested
Authorization granted
Simulated button clicked 32 times
Simulated button clicked 33 times
Downloading: [+++++/                                            ] 11 %Simulated button clicked 34 times
Downloading: [+++++++++++\                                      ] 23 %Simulated button clicked 35 times
Downloading: [+++++++++++++++++/                                ] 34 %Simulated button clicked 36 times
Downloading: [+++++++++++++++++++++++\                          ] 46 %Simulated button clicked 37 times
Downloading: [+++++++++++++++++++++++++++++\                    ] 58 %Simulated button clicked 38 times
Downloading: [+++++++++++++++++++++++++++++++++++\              ] 70 %Simulated button clicked 39 times
Downloading: [+++++++++++++++++++++++++++++++++++++++++|        ] 83 %Simulated button clicked 40 times
Downloading: [++++++++++++++++++++++++++++++++++++++++++++++++|Simulated button clicked 41 times
Downloading: [++++++++++++++++++++++++++++++++++++++++++++++++++] 100 %
Download completed
Simulated button clicked 42 times
Firmware install requested
Authorization granted
[BOOT] mbed Bootloader
[BOOT] ARM: 0000000000000000000000000000000000000000
[BOOT] OEM: 0000000000000000000000000000000000000000
[BOOT] Layout: 0 800B63C
[BOOT] Active firmware integrity check:
[BOOT] [++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++]
[BOOT] SHA256: D82A3C281A3D4EB434D1ED9D1459166EA6A50C7BF8BC315694B6CC09C5E7DF0B
[BOOT] Version: 1534927885
[BOOT] Slot 0 firmware integrity check:
[BOOT] [++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++]
[BOOT] SHA256: 2C7AD26087482D2313393A6EA60212E608C6A095A2843A80E95B5B98ABB078D1
[BOOT] Version: 1534928151
[BOOT] Update active firmware using slot 0:
[BOOT] [++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++]
[BOOT] Verify new active firmware:
[BOOT] [++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++]
[BOOT] New active firmware is valid
[BOOT] Application's start address: 0x8010400
[BOOT] Application's jump address: 0x80110C5
[BOOT] Application's stack address: 0x20040000
[BOOT] Forwarding to application...
��  � Starting Simple Mbed Cloud Client example
Connecting to the network using WiFi...
Init
Init status = 0
spif size: 4194304
spif read size: 1
spif erase size: 4096
Connected to the network successfully. IP address: 192.168.0.121

*********This line demonstrates a firmware update*********
[Simple Cloud Client] Starting developer flow
[Simple Cloud Client] Developer credentials already exist
client_status = 0
Initialized Mbed Cloud Client. Registering...
Connected to Mbed Cloud. Endpoint Name: 016560d6b3cd0000000000010010025e
Simulated button clicked 1 times
Simulated button clicked 2 times
Simulated button clicked 3 times
Simulated button clicked 4 times
Simulated button clicked 5 times
Simulated button clicked 6 times
Simulated button clicked 7 times
Simulated button clicked 8 times
Simulated button clicked 9 times
Simulated button clicked 10 times
Simulated button clicked 11 times
Simulated button clicked 12 times
Simulated button clicked 13 times
Simulated button clicked 14 times
Simulated button clicked 15 times

