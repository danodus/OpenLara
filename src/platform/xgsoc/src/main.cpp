extern "C" {
    #include "io.h"
    #include "sd_card.h"
    #include "fat/fat_filelib.h"
}

#include <stdint.h>
#include <stdio.h>
#include "SDL.h"

#include <game.h>

#define GAME_DATA_ADDR  (20*1024*1024)
#define GAME_DATA_MAGIC 0x1234ABCD

int32 fps;

bool quit = false;

const char *files[] = {
    "GYM.PKD",
    "LEVEL1.PKD",
    "LEVEL2.PKD",
    "TITLE.PKD",
    "TITLE.SCR",
    "TRACKS.AD4"
};
#define NB_FILES (sizeof(files) / sizeof(files[0]))

const void* TRACKS_AD4 = NULL;
const void* TITLE_SCR = NULL;
const void* levelData = NULL;

uint16 MEM_PAL_BG[256];

unsigned int crc32b(unsigned char *message, size_t size) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (size--) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}

int read_sector(uint32 sector, uint8 *buffer, uint32 sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_read_single_block((uint32_t)sector + i, (uint8_t *)buffer))
            return 0;
        buffer += SD_BLOCK_LEN;
    }
    return 1;
}

int write_sector(uint32 sector, uint8 *buffer, uint32 sector_count) {
    for (uint32_t i = 0; i < sector_count; ++i) {
        if (!sd_write_single_block((uint32_t)sector + i, (uint8_t *)buffer))
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

uint32 preloadFile(uint8 *data, const char *filename)
{
    uint32 size;

    char path[32];
    sprintf(path, "/data/%s", filename);

    printf("Reading %s...\n", path);
    FL_FILE *f = (FL_FILE *)fl_fopen(path, "rb");

    if (!f) {
        printf("Unable to open %s\n", path);
        return 0;
    }

    fl_fseek(f, 0, SEEK_END);
    size = fl_ftell(f);
    fl_fseek(f, 0, SEEK_SET);

    fl_fread(data, 1, size, f);
    fl_fclose(f);

    return size;
}

bool preloadData()
{
    if (MEM_READ(GAME_DATA_ADDR) == GAME_DATA_MAGIC) {
        printf("Data already loaded.\n");
        return true;
    }

    // invalidate mem data
    MEM_WRITE(GAME_DATA_ADDR, 0);

    uint8* data = (uint8 *)(GAME_DATA_ADDR + 4 + (NB_FILES * 4));

    uint32 offset = 0;
    for (size_t i = 0; i < NB_FILES; i++) {
        uint32 size = preloadFile(data, files[i]);
        if (size == 0)
            return false;
        MEM_WRITE(GAME_DATA_ADDR + 4 + (i * 4), offset);
        offset += size;
        data += size;
    }

    MEM_WRITE(GAME_DATA_ADDR, GAME_DATA_MAGIC);    

    return true;
}

const void* osLoadLevel(LevelID id)
{
    printf("osLoadLevel: /data/%s.PKD\n", (const char*)gLevelInfo[id].data);

    for (size_t i = 0; i < NB_FILES; i++) {
        char buf[32];
        sprintf(buf, "%s.PKD", (const char*)gLevelInfo[id].data);
        uint32 offset = MEM_READ(GAME_DATA_ADDR + 4 + (i * 4));
        const void* addr = (const void*)(GAME_DATA_ADDR + 4 + (NB_FILES * 4) + offset);
        if (strcmp(files[i], buf) == 0) {
            levelData = addr;
        } else if (strcmp(files[i], "TRACKS.AD4") == 0) {
            TRACKS_AD4 = addr;
        } else if (strcmp(files[i], "TITLE.SCR") == 0) {
            TITLE_SCR = addr;
        }
    }
    
    printf("Load level successful!\n");
    return (void*)levelData;
}

void blit()
{
    uint16* vram = (uint16*)MEM_VRAM;
    int i = 0;
    for (int y = 0; y < FRAME_HEIGHT; y++)
        for (int x = 0; x < FRAME_WIDTH; x++) {
            uint16 c = MEM_PAL_BG[((uint8*)fb)[i]];
            uint8 r = (c << 3);
            uint8 g = ((c >> 5) << 3);
            uint8 b = (c >> 10 << 3);
            vram[y * 320 + x] = (((uint16)r & 0b11111000) << 8) | (((uint16)g & 0b11111100) << 3) | ((uint16)b >> 3);
            i++;
        }

    // flush cache
    MEM_WRITE(CONFIG, 1);
}

void updateInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        keys |= IK_UP;
                        break;
                    case SDLK_DOWN:
                        keys |= IK_DOWN;
                        break;
                    case SDLK_LEFT:
                        keys |= IK_LEFT;
                        break;
                    case SDLK_RIGHT:
                        keys |= IK_RIGHT;
                        break;
                    case SDLK_a:
                        keys |= IK_B;
                        break;
                    case SDLK_s:
                        keys |= IK_A;
                        break;
                    case SDLK_q:
                        keys |= IK_L;
                        break;
                    case SDLK_w:
                        keys |= IK_R;
                        break;
                    case SDLK_RETURN:
                        keys |= IK_START;
                        break;
                    case SDLK_SPACE:
                        keys |= IK_SELECT;
                        break;
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        keys &= ~IK_UP;
                        break;
                    case SDLK_DOWN:
                        keys &= ~IK_DOWN;
                        break;
                    case SDLK_LEFT:
                        keys &= ~IK_LEFT;
                        break;
                    case SDLK_RIGHT:
                        keys &= ~IK_RIGHT;
                        break;
                    case SDLK_a:
                        keys &= ~IK_B;
                        break;
                    case SDLK_s:
                        keys &= ~IK_A;
                        break;
                    case SDLK_q:
                        keys &= ~IK_L;
                        break;
                    case SDLK_w:
                        keys &= ~IK_R;
                        break;
                    case SDLK_RETURN:
                        keys &= ~IK_START;
                        break;
                    case SDLK_SPACE:
                        keys &= ~IK_SELECT;
                        break;
                }
                break;
        }
    }
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

    if (!preloadData()) {
        printf("Preload data failed!\n");
        return 1;
    }

    //soundInit();
    gameInit();

    int32 lastFrame = 0;
    quit = false;
    int32 count = 0;
    int32 startTime = osGetSystemTimeMS();
    int32 loopStartTime = startTime;
    while (!quit)
    {
        updateInput();
        gameUpdate(count);
        gameRender();
        blit();
        int32 time = osGetSystemTimeMS();
        int32 frame = (time - startTime) / 33;
        count = frame - lastFrame;
        lastFrame = frame;
        int32 period = (time - loopStartTime);
        fps = (period > 0) ? (1000 / period) : 0;
        loopStartTime = time;
    }

    fl_shutdown();    

    return 0;
}
