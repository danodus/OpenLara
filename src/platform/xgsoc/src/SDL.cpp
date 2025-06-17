#include "SDL.h"
#include "io.h"

int key_avail()
{
    unsigned int ks = MEM_READ(PS2_KBD_STATUS);
    return (ks & 0x1);
}

int get_key()
{
    while (!key_avail());
    return MEM_READ(PS2_KBD_DATA);
}

static SDL_Keycode get_keycode(int scancode, bool extended_key) {
    if (extended_key) {
        switch (scancode) {
            case 0x75:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP);
            case 0x6B:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT);
            case 0x72:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN);
            case 0x74:
                return SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT);
            default:
                return SDLK_UNKNOWN;
        }
    } else {
        switch (scancode) {
            case 0x76:
                return SDLK_ESCAPE;
            case 0x16:
                return SDLK_1;
            case 0x1E:
                return SDLK_2;
            case 0x26:
                return SDLK_3;
            case 0x25:
                return SDLK_4;
            case 0x2E:
                return SDLK_5;
            case 0x36:
                return SDLK_6;
            case 0x1C:
                return SDLK_a;
            case 0x21:
                return SDLK_c;
            case 0x23:
                return SDLK_d;
            case 0x1B:
                return SDLK_s;
            case 0x1D:
                return SDLK_w;
            case 0x22:
                return SDLK_x;
            default:
                return SDLK_UNKNOWN;
        }
    }
}

int SDL_PollEvent(SDL_Event * event) {
    event->type = SDL_FIRSTEVENT;
    if (key_avail()) {
        bool extended_key = false;
        int scancode = get_key();
        if (scancode == 0xE0) {
            // extended key
            extended_key = true;
            scancode = get_key();
        }
        if (scancode == 0xF0) {
            event->type = SDL_KEYUP;
            scancode = get_key();
            event->key.keysym.sym = get_keycode(scancode, extended_key);
        } else {
            event->type = SDL_KEYDOWN;
            event->key.keysym.sym = get_keycode(scancode, extended_key);
        }
        return 1;
    }
    return 0;
}
