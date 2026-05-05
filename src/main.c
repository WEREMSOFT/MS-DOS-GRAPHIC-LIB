#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/param.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef struct AppState
{
	SDL_Texture* texture;
	void* textureData;
	double lastTime;
} AppState;

AppState appState = {0};

#define UR_SCREEN_WIDTH 320
#define UR_SCREEN_HEIGHT 240

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480


void urPutPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

#define UR_PUT_PIXEL urPutPixel
#include "universal_renderer.h"

#define Color URColor
#define PointI URPointI
#define PointF URPointF

typedef struct
{
	Uint8 r, g, b, a;
} RGBAColor;

void urPutPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	int position = (x + y * UR_SCREEN_WIDTH) % (320 * 240 * sizeof(URColor));
	((URColor *)appState.textureData)[position] = (URColor){r, g, b};
}

URSprite sprite;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

	if (!SDL_Init(SDL_INIT_VIDEO)) {
			SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
			return SDL_APP_FAILURE;
	}

	if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_SetRenderLogicalPresentation(renderer, 320, 240, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	SDL_SetRenderVSync(renderer, 1);

	sprite = urSpriteCreate("assets/sphere.bmp");

	appState.textureData = malloc(320 * 240 * sizeof(Color));

	memset(appState.textureData, 0, sizeof(Color) * 320 * 240);

	appState.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, UR_SCREEN_WIDTH, UR_SCREEN_HEIGHT);

	SDL_SetTextureScaleMode(appState.texture, SDL_SCALEMODE_NEAREST);

	appState.lastTime = ((double)SDL_GetTicks()) / 1000.0;

	*appstate = &appState;

	return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
	}

	if(event->type == SDL_EVENT_KEY_DOWN)
	{
		if(event->key.scancode == SDL_SCANCODE_ESCAPE)
		{
			return SDL_APP_SUCCESS;
		}
	}

	return SDL_APP_CONTINUE;  /* carry on with the program! */
}

void renderBackground(float);
void renderBackgroundTexture(float);

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	AppState* appState = (AppState *)appstate;

	SDL_RenderClear(renderer);

	const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
	double deltaTime = now - appState->lastTime;
	appState->lastTime = now;

	renderBackground(deltaTime);


	URColor colors[] =
	{
		UR_RED,
		UR_PURPLE,
		UR_YELLOW,
		UR_BLUE,
		UR_GREEN
	};


	URPointF mousePosition = {0};

	SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

	sprite.position.x = floor((mousePosition.x - sprite.size.x) / 2);
	sprite.position.y = floor((mousePosition.y - sprite.size.x) / 2);


	static float phase[2] = {0};
	static int start = 0;
	URPointI position[2] = {0};

	static float increment = 0;
	start = (int)increment;
	increment += 10. * deltaTime;

	phase[0] += 1. * deltaTime;
	phase[1] += 1. * deltaTime;

	int radious[2] = {30, 40};

	for(int i = 0; i < 100; i++)
	{
		URColor color = colors[(start + i) % 5];
		urDrawSquare((URPointI){.x =  radious[0] * sinf(phase[0]) + 150 - i / 2, .y = radious[0] * cosf(phase[0]) + 100 - i / 2}, (URPointI){.x = i + sinf(phase[1]) * 10, .y = i + cosf(phase[1]) * 10}, color);
	}

	urPrintString((URPointI){.x = 100, .y = 100}, "sarasa", colors[start % 5]);

	urSpriteDrawTransparentClipped(sprite);

	urPrintFPS(deltaTime);

	SDL_UpdateTexture(appState->texture, NULL, appState->textureData, 320 * sizeof(URColor));

	if(!SDL_RenderTexture(renderer, appState->texture, NULL, NULL))
	{
		printf("Error drawing texture\n");
		return SDL_APP_SUCCESS;
	}

	/* put the newly-cleared rendering on the screen. */
	SDL_RenderPresent(renderer);
	return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
/* SDL will clean up the window/renderer for us. */
}

void renderBackgroundTexture(float deltaTime)
{
	float static zoom = 1.;
	zoom += 1. * deltaTime;

	static URPointF position = {0};

	position.x = sinf(zoom) * 100 + 200;
	position.y = cosf(zoom) * 100 + 200;

	float zoomS = 1.5 + sinf(zoom * 0.9) * 0.5;
	float zoomSG = 1.5 + sinf(zoom * 0.8) * 0.6;
	float zoomSB = 1.5 + sinf(zoom) * 0.7;
	for(int x = 0; x < 320; x++)
	{
		for(int y = 0; y < 240; y++)
		{
			urPutPixel(x, y, (int)(x / zoomS + position.x) | (int)(position.y / zoomS + position.y), (int)(x / zoomSG + position.x) | (int)(y / zoomSG + position.y), (int)(x / zoomSB + position.x) | (int)(y / zoomSB + position.y));
		}
	}
}

void renderBackground(float deltaTime)
{
	int x0[3] = {100, 150, 90}, y0[3] = {100, 150, 90};
	static float phase[3] = {0};

	phase[0] += .5 * deltaTime;
	phase[1] -= .7 * deltaTime;
	phase[2] += .9 * deltaTime;

	x0[0] = sin(phase[0]) * 100 + 100;
	y0[0] = cos(phase[0]) * 100 + 100;

	x0[1] = sin(phase[1]) * 90 + 150;
	y0[1] = cos(phase[1]) * 100 + 150;

	x0[2] = sin(phase[2]) * 100 + 120;
	y0[2] = cos(phase[2]) * 110 + 120;

	for(int x = 0; x < 320; x++)
	{
		for(int y = 0; y < 240; y++)
		{
			int xb[3] = {x - x0[0], x - x0[1], x - x0[2]},
				yb[3] = {y - y0[0], y - y0[1], y - y0[2]};

			float distance2[3] = 	{
										xb[0] * xb[0] + yb[0] * yb[0],
										xb[1] * xb[1] + yb[1] * yb[1],
										xb[2] * xb[2] + yb[2] * yb[2]
									};

			int red = labs(sinf(distance2[0] * 0.0002) * 128.);
			int green = labs(sinf(distance2[1] * 0.0002) * 128.);
			int blue = labs(sinf(distance2[2] * 0.0001) * 128.);

			red += green;
			red += blue;
			red = red;

			urPutPixel(x, y, red, red, red);
		}
	}
}
