#include "config.h"

void defaultConfig(CONFIG *config) {
    config->FREQ_ACCURATE = 0;
    config->SHOW_ADVANCED = 0;
}

int loadConfig(CONFIG *config) {
	defaultConfig(config);
    int fd = sceIoOpen(CONFIG_PATH, SCE_O_RDONLY, 0777);

    char chunk[1025];
    int chunk_size = 0;
    if ((chunk_size=sceIoRead(fd, chunk, 1024)) < 2) return -1;

    char *token = strtok(chunk, "=");

    while (token && token[0] != '\n') {
        char key[30];
        sscanf(token, "%s", key);

        token = strtok(NULL, "\n");
        int value = atoi(token);
        
        token = strtok(NULL, "=");

        if (strcmp(key, "freq_accurate") == 0) {
            config->FREQ_ACCURATE = value;
            continue;
        }   
        if (strcmp(key, "show_advanced") == 0) {
            config->SHOW_ADVANCED = value;
            continue;
        }
    }

    sceIoClose(fd);

    return 0;
}


