#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


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

	if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_SetRenderLogicalPresentation(renderer, 640, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	AppState* appState = (AppState *)appstate;
	const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
	double deltaTime = now - appState->lastTime;
	appState->lastTime = now;


	/* clear the window to the draw color. */
	urClearScreen(UR_BLACK);

	URColor colors[] =
	{
		UR_RED,
		UR_PURPLE,
		UR_YELLOW,
		UR_BLUE,
		UR_GREEN
	};

	static int start = 0;

	static float increment = 0;

	increment += 10. * deltaTime;

	start = (int)increment;

	for(int i = 0; i < 100; i++)
	{
		URColor color = colors[(start + i) % 5];
		urDrawSquare((URPointI){.x = 100 - i / 2, .y = 100 - i / 2}, (URPointI){.x = i, .y = i}, color);
	}

	urPrintString((URPointI){.x = 100, .y = 100}, "sarasa", UR_YELLOW);

	urSpriteDrawTransparent(sprite);

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
