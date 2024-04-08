#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

typedef struct {
	double x;
	double y;
} vec2;

double dirToRadians(vec2 direction) {
	double pi = 3.14159265358979323846;
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
	if (window == NULL) {
		printf("Window Creation Failed. SDL_GETERROR: %s\n", SDL_GetError());
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Renderer Creation Failed. SDL_GETERROR: %s\n", SDL_GetError());
		return -1;
	}

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (texture == NULL) {
		printf("Texture Creation Failed. SDL_GETERROR: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	/*Use SDL_UpdateTexture to update window
	 * Should be updated every main cycle
	 * Pixels need to be in the same format as the texture
	 */

	vec2 pos = {2.0, 2.0};
	vec2 dir = {-1.0, 0.0};
	vec2 plane = {0.0, 1.0};

	clock_t time = 0, oldTime = 0;
	
	double oldDirX;
	double oldPlaneX;

	SDL_Event e;

	bool quit = false;
	while (!quit) {
		//oldTime = time;
		//time = clock();
		//double deltaTime = difftime(time, oldTime) / 1000.0;

		double moveSpeed = 5.0 * 0.016f;
		double rotSpeed = 3.0 * 0.016f;

		memset(pixels, 0, sizeof(pixels));

		while (SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					switch (e.key.keysym.sym) {
						case SDLK_w:
							if (map[(int)(pos.x + dir.x * moveSpeed)][(int)(pos.y)] == false) 
								pos.x += dir.x * moveSpeed;
							if (map[(int)(pos.x)][(int)(pos.y + dir.y * moveSpeed)] == false) 
								pos.y += dir.y * moveSpeed;
							break;
						case SDLK_s:
							if (map[(int)(pos.x - dir.x * moveSpeed)][(int)(pos.y)] == false) 
								pos.x -= dir.x * moveSpeed;
							if (map[(int)(pos.x)][(int)(pos.y - dir.y * moveSpeed)] == false) 
								pos.y -= dir.y * moveSpeed;
							break;
						case SDLK_d:
							oldDirX = dir.x;
							dir.x = dir.x * cos(-rotSpeed) - dir.y * sin(-rotSpeed);
							dir.y = oldDirX * sin(-rotSpeed) + dir.y * cos(-rotSpeed);
							oldPlaneX = plane.x;
							plane.x = plane.x * cos(-rotSpeed) - plane.y * sin(-rotSpeed);
							plane.y = oldPlaneX * sin(-rotSpeed) + plane.y * cos(-rotSpeed);
							break;
						case SDLK_a:
							oldDirX = dir.x;
							dir.x = dir.x * cos(rotSpeed) - dir.y * sin(rotSpeed);
							dir.y = oldDirX * sin(rotSpeed) + dir.y * cos(rotSpeed);
							oldPlaneX = plane.x;
							plane.x = plane.x * cos(rotSpeed) - plane.y * sin(rotSpeed);
							plane.y = oldPlaneX * sin(rotSpeed) + plane.y * cos(rotSpeed);
							break;
					}
				break;
			}
		}

		for (int rayCount = 0; rayCount < SCREEN_WIDTH; rayCount++) {
			//column we are working on
			double camX = 2 * rayCount / (double)SCREEN_WIDTH - 1;
			//vec of ray we are shooting
			vec2 ray = {dir.x + plane.x * camX, dir.y + plane.y * camX};
			//map position rounded to integers
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

		if (SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH*4) < 0) {
			printf("Failed to Update Texture. SDL_GETERROR: %s\n", SDL_GetError());
			return -1;
		}

		SDL_RenderClear(renderer);

		SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
