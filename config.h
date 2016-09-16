#ifndef CONFIG_H
#define CONFIG_H

#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DATA_PATH "ux0:/data/amphetamin"
#define CONFIG_PATH "ux0:/data/amphetamin/config.txt"

typedef struct CONFIG {
	int FREQ_ACCURATE;
	int SHOW_ADVANCED;
} CONFIG;

int loadConfig(CONFIG *config);

#endif
