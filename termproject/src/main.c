#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "demo.h"

#define FPS                             (60.0f)
#define MIN_FPS                         (20.0f)
#define MIN_FRAME_TIME                  (1.0f/FPS)
#define MAX_FRAME_TIME                  (1.0f/MIN_FPS)

extern SDL_Renderer* gfx_renderer;

static void cleanup(SDL_Window* window, SDL_Renderer* renderer) {
    demo_free();

    /* We need to check if a resources exists because if there
     * was a fatal error than we might be cleaning up before the
     * resource that we are freeing was ever created. */
    if (window != NULL)
        SDL_DestroyWindow(window);
    if (renderer != NULL)
        SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

static void fatal_error(const char* message, SDL_Window* window, SDL_Renderer* renderer) {
    /* Display the erorr message to the user. */
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error!", message, NULL);
    fprintf(stderr, "%s", message);

    cleanup(window, renderer);
    exit(1);
}

int main(int argc, char* argv[]) {
    /* SDL2 resources and structures. */
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event sdl_event = { 0 };
    /* Game loop and timing. */
    int done = 0;
    float delta_time = 0.0f;
    float average_fps = 0.0f;
    Uint64 start_time = 0;
    Uint64 end_time = 0;
    Uint64 timer_freq = 0;
    /* Keep track of user input. */
    int keys[4] = { 0 };

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        fatal_error("Failed to setup SDL2.", window, renderer);

    /* Create the main window that we will draw our content to. */
    window = SDL_CreateWindow(
        DEMO_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DEMO_WIDTH * DEMO_SCALE,
        DEMO_HEIGHT * DEMO_SCALE,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    /* Handle the case that the window was not created. */
    if (window == NULL)
        fatal_error("Failed to create the window.", window, renderer);

    /* Create the SDL renderer that abstracts the underlying graphics technology (OpenGL, DirectX, etc.)
     * and that we will use to draw our final 2D image to the window. */
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    /* Handle the case that the renderer was not created. */
    if (renderer == NULL)
        fatal_error("Failed to create the graphics renderer.", window, renderer);
    /* Set a virtual resolution on the renderer since this is supposed to work
     * in the same way that the old orginal 3D raycasting engines did which
     * means we need a low resolution screen. */
    SDL_RenderSetLogicalSize(renderer, DEMO_WIDTH, DEMO_HEIGHT);
    gfx_renderer = renderer;

    demo_init();

    /* Figure out of how often the high performance hardware timer ticks per second so that
     * we can accuratly control our game loop. */
    timer_freq = SDL_GetPerformanceFrequency();
    start_time = SDL_GetPerformanceCounter();

    /* Run the main game loop. This loop will try to maintain a constant frame
     * rate by sleeping the extra milliseconds. Each frame, we will calculate how
     * long the last frame took and if we did not take as long as budgeted, then
     * we will sleep and give that time back to the operating system. This will allow
     * something else to run instead of spare the user's battery. */
    while (!done) {
        if (SDL_PollEvent(&sdl_event)) {
            switch (sdl_event.type) {
            case SDL_QUIT:
                done = 1;
                break;
            case SDL_KEYDOWN:
                /* Handle the different keyboard keys that we are dealing with. */
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE)
                    done = 1;
                if (sdl_event.key.keysym.sym == SDLK_UP)
                    keys[DEMO_INPUT_UP] = 1;
                if (sdl_event.key.keysym.sym == SDLK_DOWN)
                    keys[DEMO_INPUT_DOWN] = 1;
                if (sdl_event.key.keysym.sym == SDLK_LEFT)
                    keys[DEMO_INPUT_LEFT] = 1;
                if (sdl_event.key.keysym.sym == SDLK_RIGHT)
                    keys[DEMO_INPUT_RIGHT] = 1;
                break;
            case SDL_KEYUP:
                /* Keep track of which keys are no longer held. */
                if (sdl_event.key.keysym.sym == SDLK_UP)
                    keys[DEMO_INPUT_UP] = 0;
                if (sdl_event.key.keysym.sym == SDLK_DOWN)
                    keys[DEMO_INPUT_DOWN] = 0;
                if (sdl_event.key.keysym.sym == SDLK_LEFT)
                    keys[DEMO_INPUT_LEFT] = 0;
                if (sdl_event.key.keysym.sym == SDLK_RIGHT)
                    keys[DEMO_INPUT_RIGHT] = 0;
                break;
            case SDL_WINDOWEVENT:
                /* If the window looses focus then we will clear any user input as we have no
                 * way of keeping track of what happens to the state of a key when the window
                 * does not have focus. So we just assume that the key was released. The user
                 * just press again to keep moving. */
                if (sdl_event.window.type == SDL_WINDOWEVENT_FOCUS_LOST) {
                    keys[DEMO_INPUT_UP] = 0;
                    keys[DEMO_INPUT_DOWN] = 0;
                    keys[DEMO_INPUT_LEFT] = 0;
                    keys[DEMO_INPUT_RIGHT] = 0;
                }
            /* We don't handle this event type so ignore it. */
            default:
                break;
            }
        } else {
            /* How long did the previous frame take? */
            end_time = SDL_GetPerformanceCounter();
            delta_time = (float)(end_time - start_time) / (float)timer_freq;
            /* Check to see if we have extra time left over that we can let the
             * CPU sit idle or work on something else during */
            if (delta_time < MIN_FRAME_TIME) {
                /* Figure out how many milliseconds we can sleep. */
                int sleepTime = (int)((MIN_FRAME_TIME - delta_time) * 1000);
                SDL_Delay(sleepTime);
            }

            /* Handy formula I learned from the book Programming 2D Games by Charles Kelly that
             * gives the average frame rate. Not an exact real time number but works well because
             * it is not so jumpy as the real number and gives a better overall idea of how fast
             * the game is running. */
            if (delta_time > 0.0f)
                average_fps = (average_fps * 0.99f) + (0.01f / delta_time);
            /* If the game is running really really slow, it is usually better to just stop trying to
             * run in real time. This is because at really low frame rates, the player will end up
             * moving longer distances between each frame which results in very choppy movements for
             * the player. */
            if (delta_time > MAX_FRAME_TIME)
                delta_time = MAX_FRAME_TIME;
            start_time = end_time;

            demo_tick(delta_time, keys);

            /* Always clear to black. */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            demo_draw();

            SDL_RenderPresent(renderer);
        }
    }

    cleanup(window, renderer);
    return 0;
}