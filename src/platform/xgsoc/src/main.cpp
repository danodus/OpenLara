extern "C" {
    #include "io.h"
    #include "sd_card.h"
    #include "fat/fat_filelib.h"
}

#include <stdint.h>
#include <stdio.h>

#include <game.h>

int32 fps;
const void* TRACKS_AD4 = NULL;
const void* TITLE_SCR = NULL;
const void* levelData = NULL;

uint16 MEM_PAL_BG[256];

int read_sector(uint32_t sector, uint8_t *buffer, uint32_t sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_read_single_block(sector + i, buffer))
            return 0;
        buffer += SD_BLOCK_LEN;
    }
    return 1;
}

int write_sector(uint32_t sector, uint8_t *buffer, uint32_t sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_write_single_block(sector + i, buffer))
            return 0;
        buffer += SD_BLOCK_LEN;
    }
    return 1;
}

void osSetPalette(const uint16* palette)
{
    memcpy((uint16*)MEM_PAL_BG, palette, 256 * 2);
}

int32 osGetSystemTimeMS()
{
    return MEM_READ(TIMER);
}

bool osSaveSettings()
{
    // TODO
    printf("osSaveSettings\n");
    return false;
}

bool osLoadSettings()
{
    // TODO
    printf("osLoadSettings\n");
    return false;
}

bool osCheckSave()
{
    // TODO
    printf("osCheckSave\n");
    return false;
}

bool osSaveGame()
{
    // TODO
    printf("osSaveGame\n");
    return false;
}

bool osLoadGame()
{
    // TODO
    printf("osLoadGame\n");
    return false;
}

void osJoyVibrate(int32 index, int32 L, int32 R)
{
}

const void* osLoadScreen(LevelID id)
{
    return TITLE_SCR;
}

const void* osLoadLevel(LevelID id)
{
    // level1
    char buf[32];

    if (levelData) {
        free(levelData);
        levelData = NULL;
    }

    sprintf(buf, "/data/%s.PKD", (const char*)gLevelInfo[id].data);

    printf("Reading %s...\n", buf);
    FILE *f = fl_fopen(buf, "rb");

    if (!f) {
        printf("Unable to open %s\n", buf);
        return NULL;
    }

    {
        fl_fseek(f, 0, SEEK_END);
        int32 size = fl_ftell(f);
        fl_fseek(f, 0, SEEK_SET);
        uint8* data = malloc(size);
        if (!data) {
            printf("Out of memory!");
            return NULL;
        }
        fl_fread(data, 1, size, f);
        fl_fclose(f);

        levelData = data;
    }

    // tracks
    if (!TRACKS_AD4)
    {
        printf("Reading /data/TRACKS.AD4...\n");
        FILE *f = fl_fopen("/data/TRACKS.AD4", "rb");
        if (!f) {
            printf("Unable to open /data/TRACKS.AD4\n");
            return NULL;
        }

        fl_fseek(f, 0, SEEK_END);
        int32 size = fl_ftell(f);
        fl_fseek(f, 0, SEEK_SET);
        uint8* data = malloc(size);
        if (!data) {
            printf("Out of memory!");
            return NULL;
        }        
        fl_fread(data, 1, size, f);
        fclose(f);

        TRACKS_AD4 = data;
    }

    if (!TITLE_SCR)
    {
        printf("Reading /data/TITLE.SCR...\n");        
        FILE *f = fl_fopen("/data/TITLE.SCR", "rb");
        if (!f) {
            printf("Unable to open /data/TITLE.SCR\n");
            return NULL;
        }

        fseek(f, 0, SEEK_END);
        int32 size = fl_ftell(f);
        fseek(f, 0, SEEK_SET);
        uint8* data = malloc(size);
        if (!data) {
            printf("Out of memory!");
            return NULL;
        }           
        fread(data, 1, size, f);
        fclose(f);

        TITLE_SCR = data;
    }

    printf("Load level successful!\n");
    
    return (void*)levelData;
}

int main(void)
{
    if (!sd_init()) {
        printf("SD card initialization failed.\r\n");
        return 1;
    }

    fl_init();

    // Attach media access functions to library
    if (fl_attach_media(read_sector, write_sector) != FAT_INIT_OK)
    {
        printf("Failed to init file system\r\n");
        return 1;
    }

    printf("======== OpenLara ========\n");

    gameInit();

    fl_shutdown();    

    return 0;
}
