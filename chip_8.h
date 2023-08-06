#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <SDL_render.h>
#include <SDL_opengl.h>
#include <time.h>
#include <chrono>
#include <thread>
#include "stdint.h"
#include <SDL_main.h>


#pragma once

class CPU {
private:
	


	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t stack[16];
	uint16_t index_reg = 0;
	uint8_t sp;
	uint16_t pc = 0x200;
	uint16_t opcode;
	
	public:
	CPU();
    ~CPU();
	bool loadApplication(const char* filename);
	void initialize();
	void emulatecycle();
	void fetch();
	void execute();
	bool drawFlag;
	uint8_t key[16];
	uint8_t display[64 * 32];
	uint8_t	delay_timer = 0;
	uint8_t	sound_timer = 0;
};

