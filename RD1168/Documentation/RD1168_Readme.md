* Supplied Bootloader in ../tools/ is currently built to use SPI Flash.  
   
* To build it for SD card, change main.cpp in mbed-bootloader:  
	#if MBED_CLOUD_CLIENT_UPDATE_STORAGE == ARM_UCP_FLASHIAP_BLOCKDEVICE  
	#include "SDBlockDevice.h"  
   /* initialise sd card blockdevice */  
   #if defined(MBED_CONF_APP_SPI_MOSI) && defined(MBED_CONF_APP_SPI_MISO) && \  
   defined(MBED_CONF_APP_SPI_CLK)  && defined(MBED_CONF_APP_SPI_CS)  
   SDBlockDevice sd(MBED_CONF_APP_SPI_MOSI, MBED_CONF_APP_SPI_MISO,  
   MBED_CONF_APP_SPI_CLK,  MBED_CONF_APP_SPI_CS);  
   
* Modify mbed_app.json in bootloader: Change update storage-address and remove SPI pin defines.  
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
   
* Change mbed_app.json in source:  
"update-client.storage-address"  : "(1024*1024*64)",  
   
* Once changes are made in bootloader, build it and copy the generated binary to ../tools/ directory. Rename to match the existing file name.  
* Change mbed_app.json in source and main.cpp in source to match SD card (uncomment appropriate lines in main.cpp in source) and build the binary.  
* Then combine with the bootloader using the supplied script in ../tools/ .   
* Flash the combined binary to your device. Open a terminal & observe the output.  

* Recommendation:  
Increase stack size:         "MBED_CONF_APP_MAIN_STACK_SIZE=5000", to 6K if using newer versions of Mbed OS  
