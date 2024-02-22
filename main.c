#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

typedef struct {
	double x;
	double y;
} vec2;

double dirToRadians(vec2 direction) {
	double pi = M_PI;
	double degree = atan2(direction.x, direction.y);
	return degree * pi / 180.0;
}

int map[8][8] = {
	{1,1,1,1,1,1,1,1},
	{1,0,3,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,2,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,1},
	{1,0,0,3,0,0,0,1},
	{1,1,1,1,1,1,1,1}
};

void verline(int x, int y0, int y1, uint32_t color) {
	for (int y = y0; y < y1; y++) {
		pixels[(y * SCREEN_WIDTH) + x] = color;
	}
}

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL Initialization Failed. SDL_GETERROR: %s\n", SDL_GetError());
		return -1;
	}

	//need to add error checking for these
	SDL_Window* window = SDL_CreateWindow("game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	/*Use SDL_UpdateTexture to update window
	 * Should be updated every main cycle
	 * Pixels need to be in the same format as the texture
	 */

	vec2 pos = {2.0, 2.0};
	vec2 dir = {-1.0, 0.0};
	vec2 plane = {0.0, 0.577};

	double time = 0, oldTime = 0;

	bool quit = false;
	while (!quit) {
		for (int rayCount = 0; rayCount < SCREEN_WIDTH; rayCount++) {
			double camX = 2 * rayCount / (double)SCREEN_WIDTH - 1;
			vec2 ray = {dir.x + plane.x * camX, dir.y + plane.y * camX};

			vec2 mapPos = {(int)pos.x, (int)pos.y};

			vec2 sideDist = {0, 0};

			vec2 deltaDist = {0, 0};
			deltaDist.x = (ray.x == 0) ? 1e30 : fabs(1/ray.x);
			deltaDist.y = (ray.y == 0) ? 1e30 : fabs(1/ray.y);
			double perpWallDist;

			int stepX;
			int stepY;

			int hit = 0;
			int side;

			if (ray.x < 0) {
				stepX = -1;
				sideDist.x = (pos.x - mapPos.x) * deltaDist.x;
			} else {
				stepX = 1;
				sideDist.x = (mapPos.x + 1.0 - pos.x) * deltaDist.x;
			}

			if (ray.y < 0) {
				stepY = -1;
				sideDist.y = (pos.y - mapPos.y) * deltaDist.y;
			} else {
				stepY = 1;
				sideDist.y = (mapPos.y + 1.0 - pos.y) * deltaDist.y;
			}

			while (hit == 0) {
				if (sideDist.x < sideDist.y) {
					sideDist.x += deltaDist.x;
					mapPos.x += stepX;
					side = 0;
				} else {
					sideDist.y += deltaDist.y;
					mapPos.y += stepY;
					side = 1;
				}

				if (map[(int)mapPos.x][(int)mapPos.y] > 0) hit = 1;
			}

			if (side == 0) perpWallDist = (sideDist.x - deltaDist.x);
			else perpWallDist = (sideDist.y - deltaDist.y);

			double wallHeight = (double)SCREEN_HEIGHT / perpWallDist;
			double drawStart = -wallHeight / 2.0 + (double)SCREEN_HEIGHT / 2.0;
			if (drawStart < 0) drawStart = 0;
			double drawEnd = wallHeight / 2.0 + (double)SCREEN_HEIGHT / 2.0;
			if (drawEnd >= SCREEN_HEIGHT) drawEnd = (double)SCREEN_HEIGHT - 1.0;

			uint32_t color;
			switch (map[(int)mapPos.x][(int)mapPos.y]) {
				case 1: color = 0xFF0000FF; break;
				case 2: color = 0xFF00FF00; break;
				case 3: color = 0xFFFF0000; break;
				case 4: color = 0xFFFF00FF; break;
			}

			verline(rayCount, drawStart, drawEnd, color);
		}

		SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH*4);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
