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
int32 frameIndex = 0;
int32 fpsCounter = 0;

bool quit = false;

extern uint16 fb[FRAME_WIDTH * FRAME_HEIGHT];

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
    uint32 levelSize, tracksSize, titleSize;

    if (MEM_READ(GAME_DATA_ADDR) == GAME_DATA_MAGIC) {
        printf("Data already loaded.\n");

        levelSize = MEM_READ(GAME_DATA_ADDR + 4);
        tracksSize = MEM_READ(GAME_DATA_ADDR + 8);
        titleSize = MEM_READ(GAME_DATA_ADDR + 12);
        levelData = (const void *)(GAME_DATA_ADDR + 16);
        TRACKS_AD4 = (const void *)(GAME_DATA_ADDR + 16 + levelSize);
        TITLE_SCR = (const void *)(GAME_DATA_ADDR + 16 + levelSize + tracksSize);
    } else {

        // invalidate mem data
        MEM_WRITE(GAME_DATA_ADDR, 0);

        uint8* data = (uint8 *)(GAME_DATA_ADDR + 16);

        // level1
        char buf[32];

        sprintf(buf, "/data/%s.PKD", (const char*)gLevelInfo[id].data);

        printf("Reading %s...\n", buf);
        FILE *f = fl_fopen(buf, "rb");

        if (!f) {
            printf("Unable to open %s\n", buf);
            return NULL;
        }

        fl_fseek(f, 0, SEEK_END);
        levelSize = fl_ftell(f);
        fl_fseek(f, 0, SEEK_SET);
        MEM_WRITE(GAME_DATA_ADDR + 4, levelSize);

        fl_fread(data, 1, levelSize, f);
        fl_fclose(f);

        levelData = data;
        data += levelSize;

        printf("Reading /data/TRACKS.AD4...\n");
        f = fl_fopen("/data/TRACKS.AD4", "rb");
        if (!f) {
            printf("Unable to open /data/TRACKS.AD4\n");
            return NULL;
        }

        fl_fseek(f, 0, SEEK_END);
        tracksSize = fl_ftell(f);
        fl_fseek(f, 0, SEEK_SET);
        MEM_WRITE(GAME_DATA_ADDR + 8, tracksSize);

        fl_fread(data, 1, tracksSize, f);
        fl_fclose(f);

        TRACKS_AD4 = data;
        data += tracksSize;

        printf("Reading /data/TITLE.SCR...\n");        
        f = fl_fopen("/data/TITLE.SCR", "rb");
        if (!f) {
            printf("Unable to open /data/TITLE.SCR\n");
            return NULL;
        }

        fl_fseek(f, 0, SEEK_END);
        titleSize = fl_ftell(f);
        fl_fseek(f, 0, SEEK_SET);
        MEM_WRITE(GAME_DATA_ADDR + 12, titleSize);

        fl_fread(data, 1, titleSize, f);
        fl_fclose(f);

        TITLE_SCR = data;

        MEM_WRITE(GAME_DATA_ADDR, GAME_DATA_MAGIC);

        // flush cache
        MEM_WRITE(CONFIG, 1);
    }
    
    printf("levelData=%p\n", levelData);
    printf("TRACKS_AD4=%p\n", TRACKS_AD4);
    printf("TITLE_SCR=%p\n", TITLE_SCR);
    printf("Level size: %d\n", levelSize);
    printf("Tracks size: %d\n", tracksSize);
    printf("Title size: %d\n", titleSize);
    //printf("Calculating CRC...\n");
    //printf("CRC: %x\n", crc32b(levelData, levelSize + tracksSize + titleSize));
    
    printf("Load level successful!\n");
    return (void*)levelData;
}

void blit()
{
    uint16* vram = (uint16*)MEM_VRAM;
    for (int i = 0; i < FRAME_WIDTH * FRAME_HEIGHT; i++)
    {
        uint16 c = MEM_PAL_BG[((uint8*)fb)[i]];
        uint8 r = (c << 3);
        uint8 g = ((c >> 5) << 3);
        uint8 b = (c >> 10 << 3);
        *vram = (((uint16)r & 0b11111000) << 8) | (((uint16)g & 0b11111100) << 3) | ((uint16)b >> 3);
        vram++;
    }

    // flush cache
    MEM_WRITE(CONFIG, 1);

    frameIndex++;
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
                    case 'a':
                        keys |= IK_A;
                        break;
                    case 'b':
                        keys |= IK_B;
                        break;
                    case '1':
                        keys |= IK_SELECT;
                        break;
                    case '2':
                        keys |= IK_START;
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
                    case 'a':
                        keys &= ~IK_A;
                        break;
                    case 'b':
                        keys &= ~IK_B;
                        break;
                    case '1':
                        keys &= ~IK_SELECT;
                        break;
                    case '2':
                        keys &= ~IK_START;
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

    //soundInit();
    gameInit();

    int32 lastFrameIndex = -1;

    quit = false;
    while (!quit)
    {
        updateInput();

        int32 frame = frameIndex / 2;
        //int32 delta = frame - lastFrameIndex;
        int32 delta = 2;

        // if (!delta)
        //     continue;
        lastFrameIndex = frame;

        // rumbleUpdate(delta)
        //printf(">>update\n");
        gameUpdate(delta);
        //printf("<<update\n");
        //printf(">>render\n");
        gameRender();
        //printf("<<render\n");
        blit();
        fpsCounter++;
        if (frameIndex >= 60)
        {
            frameIndex -= 60;
            lastFrameIndex -= 30;

            fps = fpsCounter;

            fpsCounter = 0;
        }        
    }

    fl_shutdown();    

    return 0;
}
