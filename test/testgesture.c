/*
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/


static const char *usage = "\n\
    i: info about devices \n\
    r: record a Gesture.(press 'r' before each new record)\n\
    s: save gestures into 'gestureSave'file\n\
    l: load 'gestureSave' file\n\
    v: enable virtual touch. Touch events are synthesized when Mouse events occur\n\
";

#include <SDL3/SDL.h>

#define SDL_GESTURE_IMPLEMENTATION 1
#include "SDL_gesture.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif


#define WIDTH  640
#define HEIGHT 480
#define BPP    4

/* MUST BE A POWER OF 2! */
#define EVENT_BUF_SIZE 256

#define VERBOSE 0

static SDL_Event events[EVENT_BUF_SIZE];
static int eventWrite;
static int colors[7] = { 0xFF, 0xFF00, 0xFF0000, 0xFFFF00, 0x00FFFF, 0xFF00FF, 0xFFFFFF };
static int quitting = 0;
static SDL_Window *g_window = NULL;


typedef struct
{
    float x, y;
} Point;

typedef struct
{
    float ang, r;
    Point p;
} Knob;

static Knob knob = { 0.0f, 0.1f, { 0.0f, 0.0f } };

static void setpix(SDL_Surface *screen, const SDL_PixelFormatDetails *format, float _x, float _y, unsigned int col)
{
    Uint32 *pixmem32;
    Uint32 colour;
    Uint8 r, g, b;
    const int x = (int)_x;
    const int y = (int)_y;
    float a;

    if ((x < 0) || (x >= screen->w) || (y < 0) || (y >= screen->h)) {
        return;
    }

    pixmem32 = (Uint32 *)screen->pixels + y * screen->pitch / BPP + x;

    SDL_memcpy(&colour, pixmem32, format->bytes_per_pixel);

    SDL_GetRGB(colour, format, NULL, &r, &g, &b);

    /* r = 0;g = 0; b = 0; */
    a = (float)((col >> 24) & 0xFF);
    if (a == 0) {
        a = 0xFF; /* Hack, to make things easier. */
    }

    a = (a == 0.0f) ? 1 : (a / 255.0f);
    r = (Uint8)(r * (1 - a) + ((col >> 16) & 0xFF) * a);
    g = (Uint8)(g * (1 - a) + ((col >> 8) & 0xFF) * a);
    b = (Uint8)(b * (1 - a) + ((col >> 0) & 0xFF) * a);
    colour = SDL_MapRGB(format, NULL, r, g, b);

    *pixmem32 = colour;
}

#if 0 /* unused */
static void
drawLine(SDL_Surface *screen, const SDL_PixelFormatDetails *format, float x0, float y0, float x1, float y1, unsigned int col)
{
    float t;
    for (t = 0; t < 1; t += (float) (1.0f / SDL_max(SDL_fabs(x0 - x1), SDL_fabs(y0 - y1)))) {
        setpix(screen, format, x1 + t * (x0 - x1), y1 + t * (y0 - y1), col);
    }
}
#endif

static void
drawCircle(SDL_Surface *screen, const SDL_PixelFormatDetails *format, float x, float y, float r, unsigned int c)
{
    float tx, ty, xr;
    for (ty = (float)-SDL_fabs(r); ty <= (float)SDL_fabs((int)r); ty++) {
        xr = (float)SDL_sqrt(r * r - ty * ty);
        if (r > 0) { /* r > 0 ==> filled circle */
            for (tx = -xr + 0.5f; tx <= xr - 0.5f; tx++) {
                setpix(screen, format, x + tx, y + ty, c);
            }
        } else {
            setpix(screen, format, x - xr + 0.5f, y + ty, c);
            setpix(screen, format, x + xr - 0.5f, y + ty, c);
        }
    }
}

static void
drawKnob(SDL_Surface *screen, const SDL_PixelFormatDetails *format, const Knob *k)
{
    drawCircle(screen, format, k->p.x * screen->w, k->p.y * screen->h, k->r * screen->w, 0xFFFFFF);
    drawCircle(screen, format, (k->p.x + k->r / 2 * SDL_cosf(k->ang)) * screen->w,
               (k->p.y + k->r / 2 * SDL_sinf(k->ang)) * screen->h, k->r / 4 * screen->w, 0);
}

static void
DrawScreen(SDL_Window *window)
{
    SDL_Surface *screen = SDL_GetWindowSurface(window);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(screen->format);
    int i;

    if (screen == NULL) {
        return;
    }

    SDL_FillSurfaceRect(screen, NULL, SDL_MapRGB(format, NULL, 75, 75, 75));

    /* draw Touch History */
    for (i = eventWrite; i < eventWrite + EVENT_BUF_SIZE; ++i) {
        const SDL_Event *event = &events[i & (EVENT_BUF_SIZE - 1)];
        const float age = (float)(i - eventWrite) / EVENT_BUF_SIZE;
        float x, y;
        unsigned int c, col;

        if ((event->type == SDL_EVENT_FINGER_MOTION) ||
            (event->type == SDL_EVENT_FINGER_DOWN) ||
            (event->type == SDL_EVENT_FINGER_UP)) {
            x = event->tfinger.x;
            y = event->tfinger.y;

            /* draw the touch: */
            c = colors[event->tfinger.fingerID % 7];
            col = ((unsigned int)(c * (0.1f + 0.85f))) | (unsigned int)(0xFF * age) << 24;

            if (event->type == SDL_EVENT_FINGER_MOTION) {
                drawCircle(screen, format, x * screen->w, y * screen->h, 5, col);
            } else if (event->type == SDL_EVENT_FINGER_DOWN) {
                drawCircle(screen, format, x * screen->w, y * screen->h, -10, col);
            }
        }
    }

    if (knob.p.x > 0) {
        drawKnob(screen, format, &knob);
    }

    SDL_UpdateWindowSurface(window);
}

static void
loop(void)
{
    union {
        SDL_Event event;
        Gesture_MultiGestureEvent mgesture;
        Gesture_DollarGestureEvent dgesture;
    } u_event;
    SDL_IOStream *stream;
    int i;

    while (SDL_PollEvent(&u_event.event)) {

        /* Record _all_ events */
        events[eventWrite & (EVENT_BUF_SIZE - 1)] = u_event.event;
        eventWrite++;

        switch (u_event.event.type) {
        case SDL_EVENT_QUIT:
           quitting = 1;
           break;

        case SDL_EVENT_KEY_DOWN:
            switch (u_event.event.key.key) {
            case SDLK_ESCAPE:
               quitting = 1;
               break;

            case SDLK_I:
            {
                SDL_TouchID *devices = SDL_GetTouchDevices(NULL);
                if (devices) {
                    for (i = 0; devices[i]; ++i) {
                        const SDL_TouchID id = devices[i];
                        const char *name = SDL_GetTouchDeviceName(id);
                        int num_fingers;
                        SDL_Finger **fingers = SDL_GetTouchFingers(id, &num_fingers);
                        SDL_free(fingers);
                        SDL_Log("Fingers Down on device %" SDL_PRIs64 " (%s): %d", id, name, num_fingers);
                    }
                    SDL_free(devices);
                }
                break;
            }

            case SDLK_R:
                Gesture_RecordGesture(-1);
                break;

            case SDLK_S:
                stream = SDL_IOFromFile("gestureSave", "w");
                SDL_Log("Wrote %i templates", Gesture_SaveAllDollarTemplates(stream));
                SDL_CloseIO(stream);
                break;

            case SDLK_L:
                stream = SDL_IOFromFile("gestureSave", "r");
                if (stream) {
                    SDL_Log("Loaded: %i", Gesture_LoadDollarTemplates(-1, stream));
                    SDL_CloseIO(stream);
                } else {
                    SDL_Log("Cannot load 'gestureSave' file");
                }
                break;

            case SDLK_V:
                /* Transform mouse event to touch events for testing without a touch screen */
                SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
                SDL_Log("SDL_HINT_MOUSE_TOUCH_EVENTS enabled");
                break;

            }
            break;

#if VERBOSE
        case SDL_EVENT_FINGER_MOTION:
            SDL_Log("Finger: %" SDL_PRIs64 ", x: %f, y: %f", u_event.event.tfinger.fingerID,
                    u_event.event.tfinger.x, u_event.event.tfinger.y);
            break;

        case SDL_EVENT_FINGER_DOWN:
            SDL_Log("Finger: %" SDL_PRIs64 " down - x: %f, y: %f",
                    u_event.event.tfinger.fingerID, u_event.event.tfinger.x, u_event.event.tfinger.y);
            break;

        case SDL_EVENT_FINGER_UP:
            SDL_Log("Finger: %" SDL_PRIs64 " up - x: %f, y: %f",
                    u_event.event.tfinger.fingerID, u_event.event.tfinger.x, u_event.event.tfinger.y);
            break;
#endif

        case GESTURE_MULTIGESTURE:
#if VERBOSE
            SDL_Log("Multi Gesture: x = %f, y = %f, dAng = %f, dR = %f",
                    u_event.mgesture.x, u_event.mgesture.y,
                    u_event.mgesture.dTheta, u_event.mgesture.dDist);
            SDL_Log("MG: numDownTouch = %i", u_event.mgesture.numFingers);
#endif

            knob.p.x = u_event.mgesture.x;
            knob.p.y = u_event.mgesture.y;
            knob.ang += u_event.mgesture.dTheta;
            knob.r += u_event.mgesture.dDist;
            break;

        case GESTURE_DOLLARGESTURE:
            SDL_Log("Gesture %" SDL_PRIs64 " performed, error: %f",
                    u_event.dgesture.gestureId, u_event.dgesture.error);
            break;

        case GESTURE_DOLLARRECORD:
            SDL_Log("Recorded gesture: %" SDL_PRIs64 "", u_event.dgesture.gestureId);
            break;
        }
    }

    DrawScreen(g_window);

#ifdef __EMSCRIPTEN__
    if (quitting) {
        emscripten_cancel_main_loop();
    }
#endif
}

int main(int argc, char *argv[])
{
    SDL_Log("%s", usage);

    g_window = SDL_CreateWindow("test gesture", WIDTH, HEIGHT, 0);
    if (g_window == NULL) {
       return -1;
    }

    Gesture_Init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!quitting) {
        loop();
        SDL_Delay(20);
    }
#endif

    Gesture_Quit();

    SDL_DestroyWindow(g_window);

    return 0;
}

