## SDL_gesture

SDL3 is removing the Gesture API that was available in SDL2, so as a migration
path, we are providing an equivalent as a single-header library that you
can drop into your SDL3-based project.

We do not make formal releases of this code; just grab the latest and drop
it into your project!

# There's a lot of files in here

You only need SDL_gesture.h ...all the other stuff, including the CMake
project files, are for the test app, which helps us make sure the header
works, but you do not need any of these files to use the library.


# Usage:

- Copy SDL_gesture.h into your project.
- Wherever you need access to this functionality, `#include` the header,
  _after_ including SDL.h.
- In **ONLY ONE PLACE** in your project, make sure you've `#defined`
  SDL_GESTURE_IMPLEMENTATION before including the header:


  ```c
  #define SDL_GESTURE_IMPLEMENTATION 1
  #include "SDL_gesture.h"
  ```

  This will make the header include not just function declarations and such
  but also its implementation code.
- To use the API, call this somewhere near startup, after SDL_Init:

  ```c
  Gesture_Init();
  ```

  and then, before you call SDL_Quit at the end of your program:

  ```c
  Gesture_Quit();
  ```

  Now you will get Gesture events from the SDL event queue.
- SDL_RecordGesture, SDL_SaveAllDollarTemplates, SDL_SaveDollarTemplate, and
  SDL_LoadDollarTemplates work as before, they are just prefixed with
  "Gesture_" instead of "SDL_". Same with event types.
- It is safe to add this to SDL2-based projects, as it will just use SDL2's
  existing API under the hood.

