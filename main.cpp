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

#include "fota.h"
#include "mbed.h"

int main(void) {
	timer(start);
	power_led(on);
	power_on_messages();
	init_n25q();
#ifdef SBL_CLEAN_BUILD
	printf(">bulk erase!\r\n");
	n25q_bulk_erase();
	wait_ms(1000);
	printf(">clean magic codes!\r\n");
	n25q_subsector_erase(0x00000000);
	wait_ms(1000);
	printf(">done!\r\n");
	timer(stop);
	power_led(off);
	while(true){

	}
#else
	if(magic_codes_match()){
		if (get_url()){
			if(!download_build()){
				sbl_reboot();
			}
		}
	}
	timer(stop);
	wait_ms(1000); // for debug output
	execute_user_application();
#endif
}
