* Introduction:
    This reference design is for the MXCHIP EMW3166 module and uses the MTB / MCB HW for implementation. The example uses "mbed-cloud-client" library that handles all the connection, registration and LWM2M resources. The example simulates a button press every 5 seconds and relays the packet to Arm Pelion IoT platform.

    The reference application also currently works on Mbed OS v5.9.5 and Mbed Cloud Client v1.3.3.

* Pre-requisites:
1. Hardware:
    * MXCHIP EMW3166 MTB / MCB with SPI flash + micro USB cable.
    * A WiFi access point.

2. Software:
    * mbed CLI - https://github.com/ARMmbed/mbed-cli
    * manifest tool - ``` pip install git+https://github.com/ARMmbed/manifest-tool --upgrade ```
    * Both the above tools will be combined into mbed CLI (via command ``` dm ``` in a later release of mbed CLI)
    * Pelion account for API keys & developer certificate.

3. Getting started:
* Import this example to your local machine with mbed CLI:
    ``` mbed import https://github.com/ARMmbed/reference-designs.git ```

* Move into the RD1168 directory ``` cd RD1168 ```
* Login to your Pelion Cloud account on a browser & follow the steps below:
    * Navigate to Device identity > Certificates
    * Select the certificate created by your account admin and click on "Download Developer C file"
    * Save the file mbed_cloud_dev_credentials.c to the location of the RD1168 directory. You have to overwrite the contents of the existing file there.

* Configure your network credentials:
    * In the mbed_app.json file in the root of the application, modify the WiFi SSID and password for your network.
    * Save the changes.

* The supplied Bootloader in ../tools/ is currently built to use SPI Flash. If you are using the SPI Flash to store the certificates, then you are good to go. If you prefer to use the SD card for the storage, follow the steps below. If not, you may skip these.
   
* To build the bootloader to use the SD card: TIP: You may want to use atleast a Class 10, 2GB SD card.
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
* Change mbed_app.json in source:
 ```   "update-client.storage-address"  : "(1024*1024*64)",  ```
   
* Once changes are made in bootloader, build it:
``` mbed compile -t ARM -m MTB_MXCHIP_EMW3166 ```

* Copy the generated binary to ../tools/ directory. Rename to match the existing file name.

* Change mbed_app.json in the root of the reference application i.e. 
"update-client.storage-address"  : "(1024*1024*64)"
* Change main.cpp in source to match SD card
* IMPORTANT: The required lines are already supplied in the code. Uncomment appropriate lines in main.cpp and build the binary.

* Combine with the bootloader using the supplied script in ../tools/ .
$> python tools/combine_bootloader_with_app.py -m MTB_MXCHIP_EMW3166 -a BUILD/MTB_MXCHIP_EMW3166/ARM/mbed-cloud-example.bin -o combined.bin

* Flash the combined binary (combined.bin) to your device. Open a terminal & observe the output.

* The device starts the example, connects to the WiFi network & after a few moments connects & registers to Pelion Cloud. It then prints out a device ID. You will need this for firmware update at a later stage.

* You can now check your simulated button counter's value in the Pelion portal as well.
* Navigate to "Device Directory" in the portal and click on your device's device ID. This will open up a new drawer on the page.
* Scroll down to LWM2M resource "Button_count" and click on it.
* A new window opens and you will be able to see the counter & a graph increment every time the resource value increments.

* Congratulations, your device is now connected to Pelion IoT platform. Please proceed to the FW update section now.

* Recommendation:  
Increase stack size:         "MBED_CONF_APP_MAIN_STACK_SIZE=5000", to 6K if using newer versions of Mbed OS  

Demonstrate a remote Firmware update:
<WIP>..
