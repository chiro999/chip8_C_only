#include <iostream>
#include "chip_8.h"
using namespace std;



unsigned char fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

    CPU::CPU() {

    }

    CPU::~CPU() {

    }


void CPU::fetch() {

}

bool CPU::loadApplication(const char* filename) {
    // Initialise
    initialize();

    printf("Loading ROM: %s\n", filename);

    // Open ROM file
    FILE* rom = fopen(filename, "rb");
    if (rom == NULL) {
        std::cerr << "Failed to open ROM" << std::endl;
        return false;
    }

    // Get file size
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    // Allocate memory to store rom
    char* rom_buffer = (char*)malloc(sizeof(char) * rom_size);
    if (rom_buffer == NULL) {
        std::cerr << "Failed to allocate memory for ROM" << std::endl;
        return false;
    }

    // Copy ROM into buffer
    size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);
    if (result != rom_size) {
        std::cerr << "Failed to read ROM" << std::endl;
        return false;
    }

    // Copy buffer to memory
    if ((4096 - 512) > rom_size) {
        for (int i = 0; i < rom_size; ++i) {
            memory[i + 512] = (uint8_t)rom_buffer[i];   // Load into memory starting
                                                        // at 0x200 (=512)
        }
    }
    else {
        std::cerr << "ROM too large to fit in memory" << std::endl;
        return false;
    }

    // Clean up
    fclose(rom);
    free(rom_buffer);

    return true;
}

void CPU::initialize() {
    //initialize regsiters and memory
    pc = 0x200;
    opcode = 0;
    index_reg = 0;
    sp = 0;

    for (int i = 0; i < 4096; ++i)
        memory[i] = 0;

    for (int i = 0; i < 16; ++i)
        stack[i] = 0;

    for (int i = 0; i < 16; ++i)
        V[i] = 0;

    for (int i = 0; i < 2048; ++i)
        display[i] = 0;

    for (int i = 0; i < 80; ++i)
        memory[i] = fontset[i];

    delay_timer = 0;
    sound_timer = 0;

    drawFlag = true;

    srand(time(NULL));
}



void CPU::emulatecycle() {
    //Fetch :one opcode is 2 bytes long. Merge using OR operation
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode) {
        case 0x00E0:
        {
            for (int i = 0; i < 2048; ++i)
                display[i] = 0;
            drawFlag = true;
            pc += 2;
        }
            break;

        case 0x00EE:
        {
            sp--;
            pc = stack[pc];
            pc += 2;
        }
            break;
        }

        break;

    case 0x1000:
    {
        pc = (opcode & 0xFFF);
        pc += 2;
    }
        break;

    case 0x2000:
    {
        //push stack
        pc = stack[pc];
        sp++;
        pc = (opcode & 0xFFF);
    }
        break;

    case 0x3000:
    {
        if (V[opcode & 0x0F00 >> 8] == (opcode & 0xFF))
            pc += 4;
        else
            pc += 2;
    }
        break;

    case 0x4000:
    {
        if (V[opcode & 0x0F00 >> 8] != (opcode & 0xFF))
            pc += 4;
        else
            pc += 2;
    }
        break;

    case 0x5000:
    {
        if (V[opcode & 0x0F00 >> 8] != V[opcode & 0x00F0 >> 4])
            pc += 4;
        else
            pc += 2;
    }
        break;


    case 0x6000: 
    {
        V[opcode & 0x0F00 >> 8] == (opcode & 0xFF);
        pc += 2;

        break;
    }

    case 0x7000:
    {
        V[opcode & 0x0F00 >> 8] += (opcode & 0xFF);
        pc += 2;
    }
        break;

    case 0x8000:
    
        switch (opcode & 0xF) {

        case 0x0:
        {
            V[opcode & 0x0F00 >> 8] == V[opcode & 0x00F0 >> 4];
            pc += 2;
        }
            break;

        case 0x1:
        {
            V[opcode & 0x0F00 >> 8] |= V[opcode & 0x00F0 >> 4];
            pc += 2;
        }
            break;

        case 0x2:
        {
            V[opcode & 0x0F00 >> 8] &= V[opcode & 0x00F0 >> 4];
            pc += 2;
        }
            break;

        case 0x3:
        {
            V[opcode & 0x0F00 >> 8] ^= V[opcode & 0x00F0 >> 4];
            pc += 2;
        }
            break;

        case 0x4:
        
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) 
                V[0xF] = 1; //carry
            
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;

            break;

        case 0x5:

            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                V[0xF] = 1; //carry
            }
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;

            break;

        case 0x6:
        {
            V[0xF] = (V[opcode & 0x0F00 >> 8] & 0x01);
            V[opcode & 0x0F00 >> 8] >>= 1;
            pc += 2;
            break;
        }
        case 0x7:
            if (V[opcode & 0x0F00 >> 8] > V[opcode & 0x00F0 >> 4])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            pc += 2;
            break;

        case 0xE:
            V[0xF] == (V[opcode & 0x0F00 >> 8] & 0x80);
            V[opcode & 0x0F00 >> 8] <<= 1;
            pc += 2;
            break;
        }

        break;

    case 0x9000:
        if (V[opcode & 0x0F00 >> 8] != V[opcode & 0x00F0 >> 4]) {
            pc += 4;
        }
        else
            pc += 2;
        break;

    case 0xA000:
        index_reg = opcode & 0xFFF;
        pc += 2;
        break;

    case 0xB000:
        pc = (opcode & 0xFFF) + V[0];
        pc += 2;

        break;

    case 0xC000:
    {
        uint8_t shuffle = rand() % (0xFF + 1);
        V[opcode & 0x0F00 >> 8] = shuffle & (opcode & 0xFF);
        pc += 2;
    }
        break;

    case 0xD000:
    {
        int width = 8;
        uint8_t x = V[opcode & 0x0F00 >> 8];
        uint8_t y = V[opcode & 0x00F0 >> 4];
        uint8_t height = (opcode & 0xF);
        uint8_t sprite;
        V[0xF] = 0;

        for (int yaxis = 0; yaxis < height; yaxis++) {
            sprite = memory[index_reg + yaxis];
            for (int xaxis = 0; xaxis < width; xaxis++) {
                if ((sprite & (0x80 >> xaxis)) != 0)
                {
                    if (display[(x + xaxis + ((y + yaxis) * 64))] == 1)
                    {
                        V[0xF] = 1;
                    }
                    display[x + xaxis + ((y + yaxis * 64))] ^= 1;
                }
            }
            drawFlag = true;
            pc += 2;
        }
    }
        break;
    case 0xE000:
        switch (opcode & 0xFF) {
        case 0x9E:
            if (key[V[opcode & 0x0F00 >> 8]] != 0)
                pc += 4;
            else
                pc += 2;
            break;

        case 0xA1:
            if (key[V[opcode & 0x0F00 >> 8]] == 0)
                pc += 4;
            else
                pc += 2;
            break;
        }
        break;

    case 0xF000:
        switch (opcode & 0xFF) {
        case 0x07:
            V[opcode & 0x0F00 >> 8] = delay_timer;
            pc += 2;
            break;

        case 0x0A:
            bool key_press = false;

            for (int i = 0; i < 16; ++i) {
                if (key[i] != 0)
                {
                    V[(opcode & 0x0F00) >> 8] = i;
                    key_press = true;
                }
            }
            if (!key_press)
                return;
            pc += 2;
        }
        break;

    case 0x15:
        delay_timer = V[opcode & 0x0F00 >> 8];
        pc += 2;
        break;

    case 0x18:
        sound_timer = V[opcode & 0x0F00 >> 8];
        pc += 2;
        break;

    case 0x1E:
        if (index_reg + V[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
            V[0xF] = 1;
        else
            V[0xF] = 0;
        index_reg += V[opcode & 0x0F00 >> 8];
        pc += 2;
        break;

    case 0x29:
        index_reg = V[opcode & 0x0F00 >> 8] * 0x5;
        pc += 2;
        break;

    case 0x33:
        memory[index_reg] = V[(opcode & 0x0F00) >> 8] / 100;
        memory[index_reg + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
        memory[index_reg + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
        pc += 2;
        break;

    case 0x55:
        for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            memory[index_reg + i] = V[i];
        index_reg += ((opcode & 0x0F00) >> 8) + 1;
        pc += 2;
        break;

    case 0x65:
        for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            V[i] = memory[index_reg + i];

        // On the original interpreter, when the operation is done, I = I + X + 1.
        index_reg += ((opcode & 0x0F00) >> 8) + 1;
        pc += 2;
        break;
    }
   

     if (delay_timer > 0)
        --delay_timer;


    if (sound_timer > 0)
    {
        if (sound_timer == 1)
            // Implement sound
            --sound_timer;
    }

}

