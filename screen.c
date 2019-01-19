#include "screen.h"




bool screen_init(size_t scale)
{

}

void screen_clear(void)
{
    for (size_t i = 0; i < SCREEN_H; ++i) {
        for (size_t j = 0; j < SCREEN_W; ++j) {
            vmem[i * SCREEN_W + j] = false;
            //clear SDL screen
        }
    }
}

bool screen_draw(uint8_t x, uint8_t y, uint8_t h)
{

}

void screen_destroy(void)
{

}