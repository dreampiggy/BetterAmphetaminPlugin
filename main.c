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

#include <psp2/types.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/appmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "blit.h"
#include "threads.h"
#include "config.h"
#include "power.h"

#define BLACK 0x00000000
#define WHITE 0x00FFFFFF
#define GREEN 0x0033CC33
#define LONG_PRESS_TIME 2000000
#define LEFT_LABEL_X 320
#define RIGHT_LABEL_X 512
#define FONT_SIZE 16

static int freq_list[] = { 111, 166, 222, 266, 333, 366, 444 };
static int freq_list_acu[] = { 41, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 111, 115, 120, 125, 130, 135, 140, 150, 155, 160, 165, 166, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 222, 225, 230, 235, 240, 250, 255, 260, 265, 266, 270, 275, 280, 285, 290, 295, 300, 305, 310, 315, 320, 325, 330, 333, 335, 340, 350, 355, 360, 365, 370, 375, 380, 385, 390, 395, 400, 405, 410, 415, 420, 425, 430, 435, 440, 444 };
#define N_FREQS (sizeof(freq_list) / sizeof(int))
#define N_FREQS_ACU (sizeof(freq_list_acu) / sizeof(int))
static uint32_t current_buttons = 0, pressed_buttons = 0;
static struct CONFIG config;
static char titleid[16] = "AMPHTAMIN";

int holdButtons(SceCtrlData *pad, uint32_t buttons) {
	if ((pad->buttons & buttons) == buttons) {
		uint64_t time_start = sceKernelGetProcessTimeWide();

		while ((pad->buttons & buttons) == buttons) {
			sceKernelDelayThread(10 * 1000);
			sceCtrlPeekBufferPositive(0, pad, 1);

			pressed_buttons = pad->buttons & ~current_buttons;
			current_buttons = pad->buttons;

			if ((sceKernelGetProcessTimeWide() - time_start) > LONG_PRESS_TIME) {
				return 1;
			}
		}
	}

	return 0;
}

int blit_thread(SceSize args, void *argp) {
	sceKernelDelayThread(5 * 1000 * 1000);
	sceAppMgrAppParamGetString(0, 12, titleid , 256);
	sceIoMkdir(DATA_PATH, 0777);
	loadConfig(&config);
	
	int menu_open = 0;
	int menu_sel = 0;

	while (1) {
		SceCtrlData pad;
		memset(&pad, 0, sizeof(SceCtrlData));
		sceCtrlPeekBufferPositive(0, &pad, 1);

		pressed_buttons = pad.buttons & ~current_buttons;
		current_buttons = pad.buttons;
		
		if (!menu_open && holdButtons(&pad, SCE_CTRL_SELECT)) {
			menu_open = 1;
			menu_sel = 0;
			pauseMainThread();			
		}

		if (menu_open) {
			if (pressed_buttons & SCE_CTRL_SELECT) {
				menu_open = 0;
				resumeMainThread();
			}

			if (pressed_buttons & SCE_CTRL_UP) {
				if (menu_sel > 0)
					menu_sel--;
			}

			if (pressed_buttons & SCE_CTRL_DOWN) {
				int menu_sel_max = 2;
				if (config.SHOW_ADVANCED) {
					menu_sel_max = 3;
				}
				if (menu_sel < menu_sel_max)
					menu_sel++;
			}

			if (pressed_buttons & SCE_CTRL_LEFT || pressed_buttons & SCE_CTRL_RIGHT) {
				int freq = 0;

				freq = getClockFrequency(menu_sel);

				if (pressed_buttons & SCE_CTRL_LEFT) {
					int i;
					if (!config.FREQ_ACCURATE) {
						for (i = 0; i < N_FREQS; i++) {
							if (freq_list[i] == freq) {
								if (i > 0)
									freq = freq_list[i - 1];
								break;
							}
						}
					} else {
						for (i = 0; i < N_FREQS_ACU; i++) {
							if (freq_list_acu[i] == freq) {
								if (i > 0)
									freq = freq_list_acu[i - 1];
								break;
							}
						}
					}
				}

				if (pressed_buttons & SCE_CTRL_RIGHT) {
					int i;
					if (!config.FREQ_ACCURATE) {
						for (i = 0; i < N_FREQS; i++) {
							if (freq_list[i] == freq) {
								if (i < N_FREQS - 1)
									freq = freq_list[i + 1];
								break;
							}
						}
					} else {
						for (i = 0; i < N_FREQS_ACU; i++) {
							if (freq_list_acu[i] == freq) {
								if (i < N_FREQS_ACU - 1)
									freq = freq_list_acu[i + 1];
								break;
							}
						}
					}
				}

				setClockFrequency(menu_sel, freq);
			}

			blit_setup();

			blit_set_color(WHITE, GREEN);
			blit_stringf(LEFT_LABEL_X, 128, "Better Amphetamin 3.2");

			blit_set_color(WHITE, menu_sel == 0 ? GREEN : BLACK);
			blit_stringf(LEFT_LABEL_X, 160, "CPU CLOCK  ");
			blit_stringf(RIGHT_LABEL_X, 160, "%-4d  MHz", getClockFrequency(0));
			blit_set_color(WHITE, menu_sel == 1 ? GREEN : BLACK);
			blit_stringf(LEFT_LABEL_X, 176, "BUS CLOCK  ");
			blit_stringf(RIGHT_LABEL_X, 176, "%-4d  MHz", getClockFrequency(1));
			blit_set_color(WHITE, menu_sel == 2 ? GREEN : BLACK);
			blit_stringf(LEFT_LABEL_X, 192, "GPU CLOCK  ");
			blit_stringf(RIGHT_LABEL_X, 192, "%-4d  MHz", getClockFrequency(2));
			if (config.SHOW_ADVANCED) { // advanced setting for GPU Crossbar
				blit_set_color(WHITE, menu_sel == 3 ? GREEN : BLACK);
				blit_stringf(LEFT_LABEL_X, 208, "XBAR CLOCK ");
				blit_stringf(RIGHT_LABEL_X, 208, "%-4d  MHz", getClockFrequency(3));
			}
			
			blit_set_color(WHITE, BLACK);
			blit_stringf(LEFT_LABEL_X, 240, "BATTERY CAP");
			blit_stringf(RIGHT_LABEL_X, 240, "%-4d  mAh", getBatteryRemCapacity());
			blit_set_color(WHITE, BLACK);
			blit_stringf(LEFT_LABEL_X, 256, "REMAINING  ");
			blit_stringf(RIGHT_LABEL_X, 256, "%-4d  min", getBatteryLifeTime());
			blit_set_color(WHITE, BLACK);
			blit_stringf(LEFT_LABEL_X, 272, "TEMPERATURE");
			blit_stringf(RIGHT_LABEL_X, 272, "%-5s Cel", getBatteryTempInCelsius());
			blit_set_color(WHITE, BLACK);
			blit_stringf(LEFT_LABEL_X, 288, "CHARGING   ");
			if (getBatteryStatus()) {
				blit_stringf(RIGHT_LABEL_X, 288, "YES %5s", getBatteryPercentage());
			} else {
				blit_stringf(RIGHT_LABEL_X, 288, "NO  %5s", getBatteryPercentage());
			}

			blit_set_color(WHITE, BLACK);
			blit_stringf(LEFT_LABEL_X, 336, "TITLEID    ");
			blit_stringf(RIGHT_LABEL_X, 336, "%9s", titleid);

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