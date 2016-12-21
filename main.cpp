/*
 * Copyright [2016] [Riccardo Pozza]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author:
 * Riccardo Pozza <r.pozza@surrey.ac.uk>
 */

#include "mbed.h"
#include "IAP.h"
#include "n25q.h"


#define START_TARGET_SECTOR    		16
#define END_TARGET_SECTOR    		29

#define PAGE_SIZE        			256
#define PAGES_PER_SECTOR			128
#define SECTOR_SIZE					32768 //32KB sectors

#define EXT_FLASH_ID_ADDR			0x80000 //after 512K
#define EXT_FLASH_ID_ADDR_LEN 		4
#define EXT_STOP_ADDR				0x90000
#define EXT_STOP_ADDR_LEN 			8
#define USER_APP_STARTING_ADDRESS	0x00010000

void execute_user_code(void){
	SCB->VTOR = 0x10000 ;
	asm("LDR 	R0, =0x10000;"
		"LDR 	SP, 	[R0];"
		"LDR	PC, [R0, #4];"
	);
}

IAP     iap;
N25Q * ExtFlash = NULL;

int main(void) {
	char mem[PAGE_SIZE];    //memory, it should be aligned to word boundary
	int *serial_number;
	int ret_val;
	int i,j,k;
	int page_address, sector_address;
	int fwid[PAGE_SIZE];
	char stop_addr[EXT_STOP_ADDR_LEN+1];
	int stop_address = 0;
	bool skip = false;
	uint8_t * ptr_page;
	Timer timings;

	timings.start();
	printf("----------------------- BOOT LOADER ---------------------------\r\n");
	printf("device-id = 0x%08X\r\n", iap.read_ID());
	serial_number = iap.read_serial();
	printf("serial# =" );
	for (i=0; i<4; i++){
		printf(" %08X", *(serial_number + i));
	}
	printf("\r\n");
	printf("cpu running @ %dkHz\r\n", SystemCoreClock/1000);
    printf("boot code vers. = 0x%08X\r\r\n", iap.read_BootVer());

    printf("----------------------- VALID ROM?? ---------------------------\r\n");
    ExtFlash = new N25Q();

	bool validextrom = false;
	page_address = EXT_FLASH_ID_ADDR;
	ExtFlash->ReadDataFromAddress(fwid, page_address, EXT_FLASH_ID_ADDR_LEN);
	for (i=0;i<EXT_FLASH_ID_ADDR_LEN;i++){
		if (fwid[i] != 0xFF){
			validextrom = true;
			break;
		}
	}
	// validextrom?
	if (validextrom){
		printf("valid external rom, magic number: %c%c%c%c!\r\n", (char) fwid[0], (char) fwid[1], (char) fwid[2], (char) fwid[3]);
		page_address = EXT_STOP_ADDR;
		ExtFlash->ReadDataFromAddress(fwid, page_address, EXT_STOP_ADDR_LEN);
		for (i=0;i<EXT_STOP_ADDR_LEN;i++){
			stop_addr[i] = (char) fwid[i];
		}
		stop_addr[8] = '\0';
		sscanf(stop_addr,"%X",&stop_address);
		printf("stop address found: %X!\r\n", stop_address);
	}
	else{
		printf("non valid external rom: %02x%02x%02x%02x!\r\n", fwid[0],fwid[1],fwid[2],fwid[3]);
	}

	printf("---------------------- INTERNAL FLASH -------------------------\r\n");
    for (i=START_TARGET_SECTOR; i<=END_TARGET_SECTOR; i++){
    	ret_val = iap.blank_check( i, i);
        printf("sector %d check, result = 0x%08X ", i, ret_val);
        if (ret_val == SECTOR_NOT_BLANK){
        	printf(" not blank! ");
        	if (validextrom){
        		printf("... prepare and erase sector %d ... ",i);
        		// erased block
        		iap.prepare(i, i);
        		ret_val = iap.erase(i,i);
        		printf("erased!!= 0x%08X", ret_val);
        	}
        	printf("\r\n");
        }
        if (ret_val == CMD_SUCCESS){
        	printf(" blank!\r\n");
        }
    }

    //clean, now write
    if (validextrom){
    	printf("----------------------- UPDATING !!! --------------------------\r\n");
    	for (i=START_TARGET_SECTOR; i<=END_TARGET_SECTOR; i++){
    		if (skip == true){
    			break;
    		}
    		for (j=0;j<PAGES_PER_SECTOR;j++){
    			sector_address = USER_APP_STARTING_ADDRESS + ((i-START_TARGET_SECTOR) * SECTOR_SIZE);
    			page_address = sector_address + (j * PAGE_SIZE);
    			printf("sector (%d | %X), page (%d | %X)\r\n",i,sector_address,j, page_address);
    			ExtFlash->ReadDataFromAddress(fwid,page_address, PAGE_SIZE);
    			ptr_page = (uint8_t *) page_address;
//    			printf("OLD DATA:\r\n");
//    			for (k=0;k<PAGE_SIZE;k++){
//    				printf("%02X ", ptr_page[k]);
//    			}
//    			printf("\r\n");
//    			printf("NEW DATA:\r\n");
    			for (k=0;k<PAGE_SIZE;k++){
    				mem[k] = (char) (fwid[k]);
//    				printf("%02X ", mem[k]);
    			}
//    			printf("\r\n");
    			// write page
    			iap.prepare(i, i);
    			printf("prepare from ram %X to flash %X \r\n", mem, page_address);
    			ret_val = iap.write  (mem, page_address,PAGE_SIZE);
    			printf("copied: Flash(0x%08X) for %d bytes. (result=0x%08X) ", page_address, PAGE_SIZE, ret_val);
    			ret_val = iap.compare(mem, page_address,PAGE_SIZE);
    			printf("| compare result = \"%s\"\r\n", ret_val ? "FAILED" : "OK" );
//    			printf("RE-WRITTEN DATA:\r\n");
//				for (k=0;k<PAGE_SIZE;k++){
//					printf("%02X ", ptr_page[k]);
//				}
//				printf("\r\n");
				if (page_address == stop_address){
					skip = true;
					printf("last page written @ %X\r\n",page_address);
					break;
				}
    		}
    	}
    	printf("----------------------- FINALIZING ! --------------------------\r\n");

    	// erasing external flash
    	ExtFlash->BulkErase();
    	printf("------------------- UPDATED SUCCESSFULLY!! --------------------\r\n");
    	printf("boot completed in %f m\r\n",(timings.read()/60));
    }
	printf("------------------------  EXITING -----------------------------\r\n");

	timings.stop();

	wait_ms(1000);

	execute_user_code();

}

