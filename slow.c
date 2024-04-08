#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

#include <math.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const int map[8][8] = {
	{1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,1,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,1,1,0,1},
	{1,0,0,0,0,1,0,1},
	{1,1,1,1,1,1,1,1}
};

double degreeToRadians(double degree) {
	double pi = 3.14159265358979323846;
	return degree * pi / 180.0;
}

int main() {
	SDL_Window* window = NULL;

	SDL_Surface* screenSurface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_ERROR: %s\n", SDL_GetError());
		return -1;
	}

	window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created! SDL_ERROR: %s\n", SDL_GetError());
		return -1;
	}

	screenSurface = SDL_GetWindowSurface(window);

	//player position
	double xPos = 2;
	double yPos = 2;

	//player direction
	double playerDirection = 180;
	double playerFov = 60;
	
	double playerSpeed = .25;
	double playerRotation = 5.0;

	//main loop flag
	bool quit = false;
	
	//SDL events
	SDL_Event e;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			} else if (e.type == SDL_KEYDOWN) {
				double playerCos = cos(degreeToRadians(playerDirection)) * playerSpeed;
				double playerSin = sin(degreeToRadians(playerDirection)) * playerSpeed;
				
				if (e.key.keysym.sym == SDLK_w) {
					xPos += playerCos;
					yPos += playerSin;
				} 
				else if (e.key.keysym.sym == SDLK_s) {
					xPos -= playerCos;
					yPos -= playerSin;
				}

				if (e.key.keysym.sym == SDLK_d) {
					playerDirection += playerRotation;
					playerDirection = fmod(playerDirection, 360.0);
				} else if (e.key.keysym.sym == SDLK_a) {
					playerDirection -= playerRotation;
					playerDirection = fmod(playerDirection, 360.0);
				}
			}
		}

		//game logic
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

		double rayAngle = playerDirection - (playerFov/2);
		
		for (int rayCount = 0; rayCount < SCREEN_WIDTH; rayCount++) {
			double rayX = xPos;
			double rayY = yPos;

			double startCos = cos(degreeToRadians(rayAngle)) / 64;
			double startSin = sin(degreeToRadians(rayAngle)) / 64;
			
			int iXPos = (int)xPos;
			int iYPos = (int)yPos;
			
			double xStep = fabs(xPos - (double)iXPos);
			double yStep = fabs(yPos - (double)iYPos);

			int wall = 0;
			while (wall == 0) {
				rayX += startCos;
				rayY += startSin;
				wall = map[(int)(rayX)][(int)(rayY)];
			}

			double distance = sqrt(pow(xPos - rayX, 2) + pow(yPos - rayY, 2));
			//printf("distance1: %f\n", distance);
	
			//fish eye distortion fix?
			distance = distance * cos(degreeToRadians(rayAngle - playerDirection));
			//printf("distance2: %f\n", distance);

			double wallHeight = SCREEN_HEIGHT/2.0/distance;
			//double wallHeight = floor((SCREEN_HEIGHT/2.0)/distance);
			
			//printf("column number: %d\n", rayCount);
			//printf("distance3: %f\n", distance);
			//printf("wallheight : %f\n", wallHeight);
			//printf("Player Direction: %f\n", playerDirection);

			SDL_Rect column;
			column.x = rayCount;
			column.y = (SCREEN_HEIGHT/2.0) - (wallHeight);
			column.w = 1;
			column.h = wallHeight*2;

			SDL_FillRect(screenSurface, &column, SDL_MapRGB(screenSurface->format, 0, 255, 0));

			rayAngle += playerFov / SCREEN_WIDTH;
		}

		SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
