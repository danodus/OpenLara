#include "game.h"

#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

int FRAME_WIDTH = 1280;
int FRAME_HEIGHT = 720;

int32 fps;

bool quit;

const void* TITLE_SCR;
const void* levelData;

uint16 MEM_PAL_BG[256];

void osSetPalette(const uint16* palette)
{
    memcpy(MEM_PAL_BG, palette, 256 * 2);
}

int32 osGetSystemTimeMS()
{
    return SDL_GetTicks();
}

bool osSaveSettings()
{
    FILE* f = fopen("settings.dat", "wb");
    if (!f) return false;
    fwrite(&gSettings, sizeof(gSettings), 1, f);
    fclose(f);
    return true;
}

bool osLoadSettings()
{
    FILE* f = fopen("settings.dat", "rb");
    if (!f) return false;
    uint8 version;
    fread(&version, 1, 1, f);
    if (version != gSettings.version) {
        fclose(f);
        return false;
    }
    fread((uint8*)&gSettings + 1, sizeof(gSettings) - 1, 1, f);
    fclose(f);
    return true;
}

bool osCheckSave()
{
    FILE* f = fopen("savegame.dat", "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

bool osSaveGame()
{
    FILE* f = fopen("savegame.dat", "wb");
    if (!f) return false;
    fwrite(&gSaveGame, sizeof(gSaveGame), 1, f);
    fwrite(&gSaveData, gSaveGame.dataSize, 1, f);
    fclose(f);
    return true;
}

bool osLoadGame()
{
    FILE* f = fopen("savegame.dat", "rb");
    if (!f) return false;

    uint32 version;
    fread(&version, sizeof(version), 1, f);

    if (SAVEGAME_VER != version)
    {
        fclose(f);
        return false;
    }

    fread(&gSaveGame.dataSize, sizeof(gSaveGame) - sizeof(version), 1, f);
    fread(&gSaveData, gSaveGame.dataSize, 1, f);
    fclose(f);
    return true;
}

void osJoyVibrate(int32 index, int32 L, int32 R) {}

const void* osLoadScreen(LevelID id)
{
    return (const void*)1; // TODO
}

const void* osLoadLevel(LevelID id)
{
    // level1
    char buf[32];

    delete[] levelData;

    sprintf(buf, "DATA/%s.PHD", (char*)gLevelInfo[id].data);

    FILE *f = fopen(buf, "rb");

    if (!f) {
        printf("level file not found!");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    int32 size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8* data = new uint8[size];
    fread(data, 1, size, f);
    fclose(f);

    levelData = data;

    return (void*)levelData;
}

void inputUpdate()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
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

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window =
        SDL_CreateWindow("OpenLara", SDL_WINDOWPOS_CENTERED_DISPLAY(1),
                         SDL_WINDOWPOS_UNDEFINED, FRAME_WIDTH, FRAME_HEIGHT, SDL_WINDOW_OPENGL);

    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    gameInit();

    int32 startTime = SDL_GetTicks() - 33;
    int32 lastFrame = 0;

    quit = false;
    while(!quit) {
        inputUpdate();

        int32 frame = (SDL_GetTicks() - startTime) / 33;
        int32 count = frame - lastFrame;
        gameUpdate(count);
        lastFrame = frame;
        clear();
        gameRender();
        renderSwap();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
