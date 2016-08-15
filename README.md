# USER APPLICATION MUST BE DOWNLOADED AT STARTING ADDRESS/ENDING ADDRESS OF SBL
# CHANGE cmsis_nvic to have:
 //#define NVIC_FLASH_VECTOR_ADDRESS (0x0)       // Initial vector position in flash
 #define NVIC_FLASH_VECTOR_ADDRESS (0x10000)     // Initial vector position in flash

#IN ADDITION, LINKER SCATTER FILE SHOULD BE MANIPULATED DIFFERENTLY FOR EACH BUILD.
/*   FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 512K */
  FLASH (rx) : ORIGIN = 0x00010000, LENGTH = 448K
  
#NB: MBED LIBRARIES MUST BE REBUILT EVERY TIME A CHANGE IN THESE FILES IS MADE