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

// NB: change also in the linker scripts and building script
#define USER_APPLICATION_ADDRESS 	0x10000
#define STRI(X)						#X
#define XTRI(X)						STRI(X)

// NB: update as storage.h in lwm2mclient
#define LWM2M_MAGIC_CODE_ADDRESS     0x00000100	// second page
#define LWM2M_FOTA_URL_ADDRESS	 	 0x00000300 // fourth page

// NB: update as such as URI len in lwm2mclient object_firmware.c
#define MAX_BUFFER_LEN 			     200

// IAP specific
#define FOTA_PAGE_SIZE     			256

enum time_operation { start, stop };
enum turn { on, off };
const char magic_code[] = MAGIC_CODE;

void init_n25q(void);
void execute_user_application(void);
void power_led(int value);
void timer(int operation);
void power_on_messages(void);
bool magic_codes_match(void);
bool get_url(void);
bool download_build(void);
unsigned int compute_crc32(char * data, unsigned int len, unsigned int crc);
void sbl_reboot(void);
void n25q_bulk_erase(void);
void n25q_subsector_erase(int startingAddress);
