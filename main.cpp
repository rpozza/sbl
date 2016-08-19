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
 * Authors:
 * Riccardo Pozza <r.pozza@surrey.ac.uk>
 */

#include "mbed.h"
#include "IAP.h"
#include "n25q.h"

#define PAGE_SIZE        			256
#define PAGES_PER_BLOCK				128 // 32KB blocks
#define START_TARGET_SECTOR    		16
#define END_TARGET_SECTOR    		29

#define EXT_FLASH_ID_ADDR			0x80000 //after 512K


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
	int *serial_number;
	int ret_val;
	int i,j,k;
	int page_address = EXT_FLASH_ID_ADDR;
	char mem[PAGE_SIZE];    //memory, it should be aligned to word boundary
	int fwid[PAGE_SIZE];
	const int fwidlen = 4;

	printf("----------------------- BOOT LOADER ---------------------------\r\n");
	printf("Device-ID = 0x%08X\r\n", iap.read_ID());
	serial_number = iap.read_serial();
	printf("Serial# =" );
	for (i=0; i<4; i++){
		printf(" %08X", *(serial_number + i));
	}
	printf("\r\n");
	printf("CPU running %dkHz\r\n", SystemCoreClock/1000);
    printf("Boot Code Vers. =0x%08X\r\r\n", iap.read_BootVer());

    printf("----------------------- VALID ROM?? ---------------------------\r\n");
    ExtFlash = new N25Q();

	bool validextrom = false;
	ExtFlash->ReadDataFromAddress(fwid, page_address, fwidlen);
	for (i=0;i<fwidlen;i++){
		if (fwid[i] != 0xFF){
			validextrom = true;
			break;
		}
	}
	// validextrom?
	if (validextrom){
		printf("Valid External Rom Found : %c%c%c%c!\r\n", fwid[0],fwid[1],fwid[2],fwid[3]);
	}
	else{
		printf("Non Valid External Rom! %x%x%x%x!\r\n", fwid[0],fwid[1],fwid[2],fwid[3]);
	}

	printf("---------------------- INTERNAL FLASH -------------------------\r\n");
    for (i=START_TARGET_SECTOR; i<=END_TARGET_SECTOR; i++){
    	ret_val = iap.blank_check( i, i);
        printf("Sector %d check, result = 0x%08X ", i, ret_val);
        if (ret_val == SECTOR_NOT_BLANK){
        	printf(" Not Blank!\r\n");
        	if (validextrom){
        		// erased block
        		iap.prepare(i, i);
        		ret_val = iap.erase(i,i);
        		printf("Erased!!= 0x%08X\r\n", ret_val);
        	}
        }
        if (ret_val == CMD_SUCCESS){
        	printf(" Blank!\r\n");
        }
    }

    //clean, now write
    if (validextrom){
    	printf("----------------------- UPDATING !!! --------------------------\r\n");
    	for (i=START_TARGET_SECTOR; i<=END_TARGET_SECTOR; i++){
    		for (j=0;j<PAGES_PER_BLOCK;j++){
    			int address = (int) sector_start_adress[i];
    			page_address += j * PAGE_SIZE;
    			ExtFlash->ReadDataFromAddress(fwid,page_address, PAGE_SIZE);
    			for (k=0;k<PAGE_SIZE;k++){
    				mem[k] = (char) fwid[k];
    			}
    			// write page
    			iap.prepare(i, i);
    			ret_val = iap.write(mem, sector_start_adress[i] +j*PAGE_SIZE,PAGE_SIZE);
    			printf("copied: Flash(0x%08X) for %d bytes. (result=0x%08X) ", mem, sector_start_adress[i] +j*PAGE_SIZE, PAGE_SIZE, ret_val);
    			ret_val = iap.compare( mem, sector_start_adress[i]+j*PAGE_SIZE, PAGE_SIZE);
    			printf("| compare result = \"%s\"\r\n", ret_val ? "FAILED" : "OK" );
    		}
    	}
    	printf("----------------------- FINALIZING ! --------------------------\r\n");
    	for (i=0;i<fwidlen;i++){
			fwid[i] = 0xFF;
		}
    	page_address = EXT_FLASH_ID_ADDR;
    	ExtFlash->ProgramFromAddress(fwid, page_address, fwidlen);
    	printf("------------------- UPDATED SUCCESSFULLY!! --------------------\r\n");
    }
	printf("------------------------  EXITING -----------------------------\r\n");

	wait_ms(1000);

	execute_user_code();

}

