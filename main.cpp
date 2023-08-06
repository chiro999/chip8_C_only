#include "chip_8.h"
#include <SDL.h>
#include <chrono>
#include <thread>

using namespace std;

CPU cpu;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture* texture;

const uint32_t fps = 60;
const uint32_t freq = 400;
const uint32_t minframetime = 1000/fps;

uint32_t screen_width = 64;
uint32_t screen_height = 32;


   uint8_t keymap[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v,
};

 


   int init_SDL() {
	   SDL_Init(SDL_INIT_EVERYTHING);

	   window = SDL_CreateWindow("$Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
	   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

	   return 0;

   }

   int main(int argc, char** argv)
   {

	   if (argc < 2)
	   {
		   printf("Usage: Chip8.exe chip8application\n\n");
		   return 1;
	   }

	   init_SDL();

	   Uint32 start_time;
	   Uint32 last_time;
	   Uint32 elapsed_time;

	   // Load game
   load:
	   if (!cpu.loadApplication(argv[1]))

		   return 1;

	   SDL_Event e;
	   bool running = true;
	   float cycles = 0;
	   last_time = SDL_GetTicks();

	   // Emulation loop
	   while (running) {
		   start_time = SDL_GetTicks();
		   elapsed_time = start_time - last_time;

		   // Process SDL events
		   while (SDL_PollEvent(&e)) {
			   if (e.type == SDL_QUIT) exit(0);

			   // Process keydown events
			   if (e.type == SDL_KEYDOWN) {
				   if (e.key.keysym.sym == SDLK_ESCAPE)
					   exit(0);

				   if (e.key.keysym.sym == SDLK_F1)
					   goto load;      // *gasp*, a goto statement!
									   // Used to reset/reload ROM

				   for (int i = 0; i < 16; ++i) {
					   if (e.key.keysym.sym == keymap[i]) {
						   cpu.key[i] = 1;
					   }
				   }
			   }
			   // Process keyup events
			   if (e.type == SDL_KEYUP) {
				   for (int i = 0; i < 16; ++i) {
					   if (e.key.keysym.sym == keymap[i]) {
						   cpu.key[i] = 0;
					   }
				   }
			   }
		   }

		   cpu.emulatecycle();
		   //play_audio(Chip8.sound_timer);

		   // If draw occurred, redraw SDL screen
		   if (cpu.drawFlag)
		   {
			   // Clear screen
			   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			   SDL_RenderClear(renderer);

			   // Draw screen
			   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			   SDL_Rect* destRect = new SDL_Rect;
			   destRect->x = 0;
			   destRect->y = 0;
			   destRect->w = 8;
			   destRect->h = 8;


			   for (int y = 0; y < 32; y++)
			   {
				   for (int x = 0; x < 64; x++)
				   {
					   if (cpu.display[(y * 64) + x] == 1)
					   {
						   destRect->x = x * 8;
						   destRect->y = y * 8;

						   SDL_RenderFillRect(renderer, destRect);
					   }
				   }
			   }

			   delete destRect;

			   SDL_RenderPresent(renderer);
			   cpu.drawFlag = false;
		   }
		   // Limit frame rate
		   //if (SDL_GetTicks() - start_time < minframetime)
		   //	SDL_Delay(minframetime - (SDL_GetTicks() - start_time));

		   last_time = start_time;
	   }

	   SDL_Quit();
   }