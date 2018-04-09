#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WIN_W 720
#define WIN_H 480

#define GAME_H 1920
#define GAME_W 1080

#define MAX_OBJECTS 10
#define ENTITIES 2
#define CAM_STEP_O 1.1
#define CAM_STEP_I 0.1
#define TIME_ACC 100

#define C_BG 6.674e-11
#define C_G 9.81
#define C_PI 3.141592

#define degrees_to_radians(deg) ((deg) * C_PI / 180.0)
#define radians_to_degrees(rad) ((rad) * 180.0 / C_PI)

Uint64 time_now;
Uint64 time_last = 0;
double delta_time = 0;

typedef struct {
    float x;
    float y;
} Vector2;

typedef struct {
    float w;
    float h;
} Boundb_x;

typedef struct {
    float r;
} BoundCircle;

typedef struct {
    Vector2 pos;
    float viewport_w;
    float viewport_h;
    float scale;
} Camera;

typedef enum {
    Collsion_b_x,
    Collsion_Circle
} CollisionType;

typedef struct {
    SDL_Texture *texture;
    float mass;
    float rot;
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    Vector2 force;
    Boundb_x bb;
    BoundCircle bc;
    CollisionType ct;
} Object;

typedef struct {
    Object *objects[MAX_OBJECTS];
} GameState;

Object *object_create(char *path, SDL_Renderer *render, SDL_Window *window, float mass)
{
    Object *plnt = malloc(sizeof(Object));
    SDL_Surface *bmp = IMG_Load(path);
    if (!bmp) {
        perror("Cannot find image");
        SDL_Quit();
    }

    plnt->texture = SDL_CreateTextureFromSurface(render, bmp);
    SDL_FreeSurface(bmp);
    if (!plnt->texture){
        SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    Vector2 tmp;
    tmp.x = 0;
    tmp.y = 0;
    plnt->mass = mass;
    plnt->pos = tmp;
    plnt->vel = tmp;
    plnt->acc = tmp;
    plnt->force = tmp;

    return plnt;
}

int object_destroy(Object *plnt)
{
    if(!plnt) {
        SDL_DestroyTexture(plnt->texture);
        free(plnt);
        return 0;
    }
    else
        return -1;
}

Vector2 vector2_add(Vector2 a, Vector2 b) 
{
    Vector2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}


bool valueInRange(int value, int min, int max)
{ 
    return (value >= min) && (value <= max); 
}

int intersect_rects(Object *a, Object *b)
{
    float a_x = a->pos.x;
    float a_y = a->pos.y;
    float b_x = b->pos.x;
    float b_y = b->pos.y;

    float a_w = a->bb.w;
    float a_h = a->bb.h;
    float b_w = b->bb.w;
    float b_h = b->bb.h;

    bool xOverlap = valueInRange(a_x, b_x, b_x + b_w) ||
        valueInRange(b_x, a_x, a_x + a_w);

    bool yOverlap = valueInRange(a_y, b_y, b_y + b_h) ||
        valueInRange(b_y, a_y, a_y + a_h);
    
    //printf("A-> x:%f y:%f, w:%f h:%f\n", a_x, a_y, a_w, a_h);
    //printf("B-> x:%f y:%f, w:%f h:%f\n", b_x, b_y, b_w, b_h);

    return xOverlap && yOverlap;
}

int update_physics(GameState *gs)
{
    Vector2 tmp;
    tmp.x = 0;
    tmp.y = 0;
    for(int i = 0; i < ENTITIES; i++) {
        gs->objects[i]->force = tmp;
    }

    for(int i = 0; i < ENTITIES; i++) {
        for(int j = i; j < ENTITIES; j++) {
            Object *plnt1 = gs->objects[i];
            Object *plnt2 = gs->objects[j];
            float x_diff = plnt2->pos.x - plnt1->pos.x;
            float y_diff = plnt2->pos.y - plnt1->pos.y;

            if(i != j) {
                double force_x = 0;
                double force_y = 0;
                if(x_diff != 0)
                    force_x = C_BG * plnt1->mass * plnt2->mass / x_diff;
                if(y_diff != 0)
                    force_y = C_BG * plnt1->mass * plnt2->mass / y_diff;
                printf("Object %d and %d will feel x:%.15f y:%.15f N\n", i, j, force_x, force_y);
                gs->objects[i]->force.x += force_x;
                gs->objects[i]->force.y += force_y;
                gs->objects[j]->force.x -= force_x;
                gs->objects[j]->force.y -= force_y;
            }
        }
    }
    printf("R->x: %.15f y:%.15f a.x: %.15f a.y: %.15f\n", gs->objects[0]->pos.x, gs->objects[0]->pos.y, gs->objects[0]->acc.x, gs->objects[0]->acc.y);
    printf("E->x: %.15f y:%.15f a.x: %.15f a.y: %.15f\n", gs->objects[1]->pos.x, gs->objects[1]->pos.y, gs->objects[1]->acc.x, gs->objects[1]->acc.y);

    time_last = time_now;
    time_now = SDL_GetPerformanceCounter();

    delta_time = ((double)((time_now - time_last) * 1000 / SDL_GetPerformanceFrequency())) * TIME_ACC;

    for(int i = 0; i < ENTITIES; i++) {
        //gs->objects[i]->force.y += 0.002;
        //gs->objects[i]->force.x += 0.002;

        gs->objects[i]->acc.x = gs->objects[i]->force.x / gs->objects[i]->mass;
        gs->objects[i]->vel.x += gs->objects[i]->acc.x * delta_time;
        gs->objects[i]->pos.x += gs->objects[i]->vel.x * delta_time;

        gs->objects[i]->acc.y = gs->objects[i]->force.y / gs->objects[i]->mass;
        gs->objects[i]->vel.y += gs->objects[i]->acc.y * delta_time;
        gs->objects[i]->pos.y += gs->objects[i]->vel.y * delta_time;
    }

    if(intersect_rects(gs->objects[0], gs->objects[1])) {
        gs->objects[1]->acc = tmp;
        gs->objects[1]->vel = tmp;
        gs->objects[0]->acc = tmp;
        gs->objects[0]->vel = tmp;
        printf("Collision\n");
    }

    //printf("R->x: %.15f y:%.15f F.x: %.15f F.y: %.15f\n", gs->objects[0]->pos.x, gs->objects[0]->pos.y, gs->objects[0]->force.x, gs->objects[0]->force.y);

    return 0;
}

int update_render(GameState *gs, SDL_Renderer *render, Camera *camera)
{
    SDL_RenderClear(render);

    for(int i = 0; i < ENTITIES; i++) {
        bool brender = false;

        SDL_Rect e;
        e.w = gs->objects[i]->bb.w * camera->scale;
        e.h = gs->objects[i]->bb.h * camera->scale;
        //printf("W:%d H:%d\n", e.w, e.h);
        //printf("Camera Scale %f\n", camera->scale);

        e.x = camera->scale *(gs->objects[i]->pos.x - camera->pos.x );
        e.y = camera->scale *(gs->objects[i]->pos.y - camera->pos.y );
        
        //if(gs->objects[i]->pos.x + gs->objects[i]->bb.w/2 < camera->pos.x + camera->viewport_w / 2 && gs->objects[i]->pos.x > camera->pos.x - camera->viewport_w / 2)
        if(e.x + e.w > 0 && e.x < WIN_W && e.y + e.h > 0 && e.y < WIN_H) {
            brender = true;
        }
        if(brender)
        {

            //SDL_RenderCopy(render, gs->objects[0]->texture, NULL, &e);
            SDL_SetRenderDrawColor(render, 0,255,0,255);
            SDL_RenderFillRect(render, &e);
            SDL_SetRenderDrawColor(render, 0,0,0,255);
            SDL_RenderCopyEx(render, gs->objects[i]->texture, NULL, &e, radians_to_degrees(gs->objects[i]->rot), NULL, SDL_FLIP_NONE);
        }
    }

    //printf("Camera X:%f Y:%f\n", camera->pos.x, camera->pos.y);
    //printf("Object X:%f Y:%f\n", gs->objects[1]->pos.x, gs->objects[1]->pos.y);

    SDL_RenderPresent(render);
    return 0;
}

int main( int argc, char* args[] )
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
        perror("Cannot init SDL");

    SDL_Window *window = SDL_CreateWindow("Hello World!", 100, 100, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if(!window)
        perror("Cannot open a window");

    SDL_Renderer *render = SDL_CreateRenderer(window, -1, 0);
    if(!render)
        perror("Cannot create a render");

    GameState game_state;

    Object *earth = object_create("res/planet2.png", render, window, 300);
    Object *rocket = object_create("res/rocket.png", render, window, 1);
    //Object *mars = object_create("res/planet2.png", render, window, 6.4e23);
    //Object *venus = object_create("res/planet2.png", render, window, 1.9e27);

    earth->pos.x = 0;
    earth->pos.y = 0;
    earth->bb.w = 200;
    earth->bb.h = 200;

    rocket->pos.x = 300;
    rocket->pos.y = 100;
    rocket->rot = C_PI/3;
    rocket->bb.w = 20;
    rocket->bb.h = 20;

    game_state.objects[0] = rocket;
    game_state.objects[1] = earth;
    //game_state.objects[2] = venus;

    time_now = SDL_GetPerformanceCounter();


    bool bquit = false;

    Camera camera;
    camera.pos.x = 0;
    camera.pos.y = 0;
    camera.scale = 1;
    camera.viewport_w = WIN_W;
    camera.viewport_h = WIN_H;


    while(!bquit) {
        update_physics(&game_state);
        update_render(&game_state, render, &camera);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                bquit = true;
                break;
            }
            else if (event.key.keysym.sym == SDLK_ESCAPE)
                update_physics(&game_state); 

            switch(event.key.keysym.sym) { 
                case SDLK_SPACE: 
                    game_state.objects[0]->force.x = 10;
                    game_state.objects[0]->force.y = 10;
                    printf("Boost off F.x:%f\n", game_state.objects[0]->force.x);
                    break;
                case SDLK_RIGHT:
                    camera.pos.x -= 20;
                    break;
                case SDLK_LEFT:
                    camera.pos.x += 20;
                    break;
                case SDLK_UP:
                    camera.pos.y += 20;
                    break;
                case SDLK_DOWN:
                    camera.pos.y -= 20;
                    break;
                default:
                    break;
            }

            if(event.type == SDL_MOUSEWHEEL) {

                if(event.wheel.y > 0)
                    camera.scale += CAM_STEP_I;
                else if(event.wheel.y < 0) {
                    camera.scale /= CAM_STEP_O;
                }
            }
        }

    }

    object_destroy(earth);
    object_destroy(rocket);

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
