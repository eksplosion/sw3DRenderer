#include <SDL2/SDL.h>
#include <png.h>
#include <stdio.h>
#include <math.h>
#include "types.h"
#include "global.h"
#include "mesh.h"
#include "render.h"
#include "texture.h"

Uint32 ticks_delta = 0;

SDL_Window   *g_window;
SDL_Renderer *g_renderer;
SDL_Texture  *g_window_texture;
SDL_Event g_event;

texture_group g_textures;

int g_state_depth_view = 0;

int color = 0;

Uint32 window_width = 640;
Uint32 window_height = 480;

Uint32 *pixels = NULL;
float *depth = NULL;

float matrix_rot[3][3][3] = { 0.0f };

matrix projection = { 0.0f };

vec3 camera_position = { 0.0f, 0.0f, -5.f };
vec3 camera_rotation = { 0.0f };

int window_focused;
mesh cube, cube2, teapot, teapot_uv, wall;

Uint32 HSVtoRGB(int H, double S, double V)
{
	double C = S * V;
	double X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
	double m = V - C;
	double Rs, Gs, Bs;

	if(H >= 0 && H < 60) {
		Rs = C;
		Gs = X;
		Bs = 0;
	}
	else if(H >= 60 && H < 120) {
		Rs = X;
		Gs = C;
		Bs = 0;
	}
	else if(H >= 120 && H < 180) {
		Rs = 0;
		Gs = C;
		Bs = X;
	}
	else if(H >= 180 && H < 240) {
		Rs = 0;
		Gs = X;
		Bs = C;
	}
	else if(H >= 240 && H < 300) {
		Rs = X;
		Gs = 0;
		Bs = C;
	}
	else {
		Rs = C;
		Gs = 0;
		Bs = X;
	}

    return
        0xff000000 + 
    	((int)((Rs + m) * 255) << 16) +
	    ((int)((Gs + m) * 255) << 8) +
    	(Bs + m) * 255;
}

void window_resize(Uint32 width, Uint32 height)
{
    printf("Window resized to %dx%d pixels\n", width, height);
    window_width  = width;
    window_height = height;

    if(g_window_texture != NULL)
        SDL_DestroyTexture(g_window_texture);
    g_window_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    pixels = realloc(pixels, sizeof(Uint32) * width * height);
    depth = realloc(depth, sizeof(float) * width * height);

    float near = .1f;
    float far = 100.f;
    float fov = 90.f;
    float aspectRatio = (float)window_height / window_width;
    float fovRad = 1.f / tanf(fov / 2.f * DEG_TO_RAD);

    projection = (matrix){
        aspectRatio * fovRad, 0.f   , 0.f                         , 0.f,
        0.f                 , fovRad, 0.f                         , 0.f,
        0.f                 , 0.f   , far / (far - near)          , 1.f,
        0.f                 , 0.f   , (-far * near) / (far - near), 0.f
    };
}

void start()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_window = SDL_CreateWindow("Software 3D renderer", 0, 0, window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    g_window_texture = NULL;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    window_focused = 1;
    window_resize(window_width, window_height);

    texture_group_texture_add(&g_textures, "assets/pattern.png");
    mesh_create(&cube, 0);
    mesh_create(&wall, 0);
    mesh_create(&teapot, 6320);
    mesh_create(&teapot_uv, 6320);
    mesh_load("assets/cube.obj", &cube, 0xff0000ff, -1);
    mesh_load("assets/wall.obj", &wall, 0xff0000ff, -1);
    mesh_load("assets/teapot.obj", &teapot, 0xffffffff, -1);
    mesh_load("assets/teapot_uv.obj", &teapot_uv, 0xffffffff, 0);

    teapot_uv.pos.x = 6.f;
    cube.pos.y = 4.f;
    cube2 = cube;
    mesh_create(&cube2, 12);
    cube2.pos.x += 6.f;
    cube2.pos.y = 4.f;

    for(int i = 0; i < cube.size; i++)
        mesh_triangle_add(&cube2, cube.t[i]);
}

int update()
{
    while (SDL_PollEvent(&g_event))
    {
        if(g_event.type == SDL_QUIT)
            return 0;
        if(g_event.window.event == SDL_WINDOWEVENT_RESIZED)
            window_resize(g_event.window.data1, g_event.window.data2);
        if(g_event.type == SDL_MOUSEMOTION && window_focused)
        {
            camera_rotation.x -= 0.3 * g_event.motion.yrel;
            camera_rotation.y += 0.3 * g_event.motion.xrel;
            if(camera_rotation.x >   90.f) camera_rotation.x =   90.f;
            if(camera_rotation.x <  -90.f) camera_rotation.x =  -90.f;
            if(camera_rotation.y >  180.f) camera_rotation.y -=  360.f;
            if(camera_rotation.y < -180.f) camera_rotation.y +=  360.f;
        }
        if(g_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
        {
            window_focused = 1;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        if(g_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            window_focused = 0;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }

    float z = camera_rotation.z * DEG_TO_RAD;
    float y = -camera_rotation.y * DEG_TO_RAD;
    float x = -camera_rotation.x * DEG_TO_RAD;

    matrix_rot[X][0][0] = 1;
    matrix_rot[X][1][1] = cos(x);
    matrix_rot[X][1][2] = sin(x);
    matrix_rot[X][2][1] = -sin(x);
    matrix_rot[X][2][2] = cos(x);
    matrix_rot[Y][0][0] = cos(y);
    matrix_rot[Y][0][2] = sin(y);
    matrix_rot[Y][1][1] = 1;
    matrix_rot[Y][2][0] = -sin(y);
    matrix_rot[Y][2][2] = cos(y);
    matrix_rot[Z][0][0] = cos(z);
    matrix_rot[Z][0][1] = sin(z);
    matrix_rot[Z][1][0] = -sin(z);
    matrix_rot[Z][1][1] = cos(z);
    matrix_rot[Z][2][2] = 1;


    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
    float theta = camera_rotation.y * DEG_TO_RAD;
    if (keyboard[SDL_SCANCODE_ESCAPE])
    {
        window_focused = 0;
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    if (keyboard[SDL_SCANCODE_Q])
        // Quit program
        return 0;

    float speed = 1.f;
    if (keyboard[SDL_SCANCODE_LCTRL])
        speed = 0.2f;

    vec3 mov = { 0.f, 0.f, 0.f };

    if (keyboard[SDL_SCANCODE_D])
    {
        mov.x += 0.01f * cosf(theta) * ticks_delta * speed;
        mov.z += 0.01f * -sinf(theta) * ticks_delta * speed;
    }
    if (keyboard[SDL_SCANCODE_A])
    {
        mov.x -= 0.01f *  cosf(theta) * ticks_delta * speed;
        mov.z -= 0.01f * -sinf(theta) * ticks_delta * speed;
    }
    if (keyboard[SDL_SCANCODE_W])
    {
        mov.x += 0.01f * sinf(theta) * ticks_delta * speed;
        mov.z += 0.01f * cosf(theta) * ticks_delta * speed;
    }
    if (keyboard[SDL_SCANCODE_S])
    {
        mov.x -= 0.01f * sinf(theta) * ticks_delta * speed;
        mov.z -= 0.01f * cosf(theta) * ticks_delta * speed;
    }
    if (keyboard[SDL_SCANCODE_SPACE])
    {
        mov.y += 0.005f * ticks_delta * speed;
    }
    if (keyboard[SDL_SCANCODE_LSHIFT])
    {
        mov.y -= 0.005f * ticks_delta * speed;
    }

    camera_position.x += mov.x;
    camera_position.y += mov.y;
    camera_position.z += mov.z;

    static int key_holding_v = 0;
    if (keyboard[SDL_SCANCODE_V] && key_holding_v == 10)
    {
        key_holding_v = 0;
        g_state_depth_view = 1 - g_state_depth_view;
    }
    else key_holding_v += (key_holding_v < 10);

    memset(pixels, 128, sizeof(Uint32) * window_width * window_height);
    for(int i = 0; i < window_width * window_height; i++)
        depth[i] = 0.f;

    color++;
    color %= 360;

    Uint32 rgb = HSVtoRGB(color, 1.0, 1.0);

    for(int i = 0; i < cube.size; i++)
        cube.t[i].color = rgb;

    for(int i = 0; i < wall.size; i++)
        wall.t[i].color = rgb;

    cube.rotation.y += 0.05f * ticks_delta;
    cube.rotation.x += 0.02f * ticks_delta;
    cube.rotation.z += 0.01f * ticks_delta;
    cube.processed = 0;
    cube2.rotation.y += 0.05f * ticks_delta;
    cube2.rotation.x -= 0.02f * ticks_delta;
    cube2.rotation.z += 0.01f * ticks_delta;
    cube2.processed = 0;
    wall.rotation.y += 0.02f * ticks_delta;
    wall.processed = 0;

    mesh_render(&cube);
    mesh_render(&cube2);
    mesh_render(&wall);
    mesh_render(&teapot);
    mesh_render(&teapot_uv);

    if(g_state_depth_view)
        for(int i = 0; i < window_width * window_height; i++)
        {
            float c = depth[i];
            if(c > 1.f) c = 1.f;
            c *= 255;
            pixels[i] = ((int)c << 16) + ((int)c << 8) + (int)c;
        }

    SDL_UpdateTexture(g_window_texture, NULL, pixels, sizeof(Uint32) * window_width);
    SDL_RenderCopy(g_renderer, g_window_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
    return 1;
}

int main()
{
    start();
    int running = 1;
    Uint32 ticks, oldTicks, frames = 0, thousand = 1000;
    while(running)
    {
        oldTicks = SDL_GetTicks();
        running = update();
        ticks = SDL_GetTicks();
        ticks_delta = ticks - oldTicks;
        frames++;

        if(ticks >= thousand)
        {
            float ms = (float)(ticks - thousand + 1000) / frames;
            float fps = 1000.f / ms;
            printf("FPS: %.2f %.2f ms avg.\n", fps, ms);
            frames = 0;
            thousand += 1000;
        }
    }
    return 0;
}
