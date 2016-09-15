/*
	Vitamin
	Copyright (C) 2016, Team FreeK (TheFloW, Major Tom, mr. gas)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	
	###################################
	# !!! Made better by BeatPlay !!! #
	###################################
*/

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/moduleinfo.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "blit.h"

#define BLACK 0x00000000
#define RED 0x0033CC33
static int freq_list[] = { 41, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 111, 115, 120, 125, 130, 135, 140, 150, 155, 160, 165, 166, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 222, 225, 230, 235, 240, 250, 255, 260, 265, 266, 270, 275, 280, 285, 290, 295, 300, 305, 310, 315, 320, 325, 330, 333, 335, 340, 350, 355, 360, 365, 370, 375, 380, 385, 390, 395, 400, 405, 410, 415, 420, 425, 430, 435, 440, 444 };
#define N_FREQS (sizeof(freq_list) / sizeof(int))
static int bright_list[] = {}
#define N_BRIGHT (sizeof(bright_list)) / sizeof(int))

static uint32_t current_buttons = 0, pressed_buttons = 0;

int holdButtons(SceCtrlData *pad, uint32_t buttons, uint64_t time) {
	if ((pad->buttons & buttons) == buttons) {
		uint64_t time_start = sceKernelGetProcessTimeWide();

		while ((pad->buttons & buttons) == buttons) {
			sceKernelDelayThread(10 * 1000);
			sceCtrlPeekBufferPositive(0, pad, 1);

			pressed_buttons = pad->buttons & ~current_buttons;
			current_buttons = pad->buttons;

			if ((sceKernelGetProcessTimeWide() - time_start) >= time) {
				return 1;
			}
		}
	}

	return 0;
}

/*
 * Tricky way to freeze main thread, we set our plugin priority to 0 (max)
 * and we start two threads with 0 priority in order to get VITA scheduler
 * to always reschedule our threads instead of main one
 */
volatile int term_stubs = 0;
int stub_thread(SceSize args, void *argp){
	for (;;){if (term_stubs) sceKernelExitDeleteThread(0);}	
}
void pauseMainThread(){
	sceKernelChangeThreadPriority(0, 0x0);
	int i;
	term_stubs = 0;
	for (i=0;i<2;i++){
		SceUID thid = sceKernelCreateThread("thread", stub_thread, 0x0, 0x40000, 0, 0, NULL);
		if (thid >= 0)
			sceKernelStartThread(thid, 0, NULL);
	}
}
void resumeMainThread(){
	term_stubs = 1;
	sceKernelChangeThreadPriority(0, 0x40);
}
/*
 * END OF PAUSE/RESUME THREADS TRICK
*/

int blit_thread(SceSize args, void *argp) {
	sceKernelDelayThread(5 * 1000 * 1000);
	
	int menu_open = 0;
	int menu_sel = 0;

	while (1) {
		SceCtrlData pad;
		memset(&pad, 0, sizeof(SceCtrlData));
		sceCtrlPeekBufferPositive(0, &pad, 1);

		pressed_buttons = pad.buttons & ~current_buttons;
		current_buttons = pad.buttons;
		
		if (!menu_open && holdButtons(&pad, SCE_CTRL_SELECT, 1 * 1000 * 1000)) {
			menu_open = 1;
			menu_sel = 0;
			pauseMainThread();			
		}

		if (menu_open) {
			if (pressed_buttons & SCE_CTRL_SELECT){
				menu_open = 0;
				resumeMainThread();
			}

			if (pressed_buttons & SCE_CTRL_UP) {
				if (menu_sel > 0)
					menu_sel--;
			}

			if (pressed_buttons & SCE_CTRL_DOWN) {
				if (menu_sel < 2)
					menu_sel++;
			}

			if (pressed_buttons & SCE_CTRL_LEFT || pressed_buttons & SCE_CTRL_RIGHT) {
				int freq = 0;

				switch (menu_sel) {
					case 0:
						freq = scePowerGetArmClockFrequency();
						break;
						
					case 1:
						freq = scePowerGetBusClockFrequency();
						break;
						
					case 2:
						freq = scePowerGetGpuClockFrequency();
						break;
				}

				if (pressed_buttons & SCE_CTRL_LEFT) {
					int i;
					for (i = 0; i < N_FREQS; i++) {
						if (freq_list[i] == freq) {
							if (i > 0)
								freq = freq_list[i - 1];
							break;
						}
					}
				}

				if (pressed_buttons & SCE_CTRL_RIGHT) {
					int i;
					for (i = 0; i < N_FREQS; i++) {
						if (freq_list[i] == freq) {
							if (i < N_FREQS - 1)
								freq = freq_list[i + 1];
							break;
						}
					}
				}

				switch (menu_sel) {
					case 0:
						scePowerSetArmClockFrequency(freq);
						break;
						
					case 1:
						scePowerSetBusClockFrequency(freq);
						break;
						
					case 2:
						scePowerSetGpuClockFrequency(freq);
						break;
				}
			}

			blit_setup();
			blit_set_color(0x00FFFFFF, 0x0033CC33);
			blit_stringf(336, 128, "Better Amphetamin");

			blit_set_color(0x00FFFFFF, menu_sel == 0 ? RED : BLACK);
			blit_stringf(336, 160, "CPU CLOCK");
			blit_stringf(496, 160, "%d MHZ", scePowerGetArmClockFrequency());
			blit_set_color(0x00FFFFFF, menu_sel == 1 ? RED : BLACK);
			blit_stringf(336, 176, "BUS CLOCK");
			blit_stringf(496, 176, "%d MHZ", scePowerGetBusClockFrequency());
			blit_set_color(0x00FFFFFF, menu_sel == 2 ? RED : BLACK);
			blit_stringf(336, 192, "GPU CLOCK");
			blit_stringf(496, 192, "%d MHZ", scePowerGetGpuClockFrequency());
			
			blit_set_color(0x00FFFFFF, menu_sel == 2 ? BLACK : BLACK);
			blit_stringf(336, 224, "BATTERY  ");
			blit_stringf(496, 224, "%d mAh", scePowerGetBatteryRemainCapacity());
			if (scePowerIsBatteryCharging() == 1) {
				blit_set_color(0x00FFFFFF, menu_sel == 2 ? BLACK : BLACK);
				blit_stringf(336, 240, "CHARGING ");
				blit_stringf(496, 240, "Yes    ");
			}else {
				blit_set_color(0x00FFFFFF, menu_sel == 2 ? BLACK : BLACK);
				blit_stringf(336, 240, "CHARGING ");
				blit_stringf(496, 240, "No     ");
			}
			blit_set_color(0x00FFFFFF, menu_sel == 2 ? BLACK : BLACK);
			blit_stringf(336, 256, "REMAINING");
			blit_stringf(496, 256, "%d min", scePowerGetBatteryLifeTime());
		}
		sceDisplayWaitVblankStart();
	}

	return 0;
}

int _start(SceSize args, void *argp) {
	SceUID thid = sceKernelCreateThread("blit_thread", blit_thread, 0x40, 0x40000, 0, 0, NULL);
	if (thid >= 0)
		sceKernelStartThread(thid, 0, NULL);

	return 0;
}