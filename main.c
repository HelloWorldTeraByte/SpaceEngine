#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

#define WIN_W 640
#define WIN_H 480

#define MAX_OBJECTS 10

#define C_BG 6.674e-11
#define C_G 9.81

typedef struct {
    float x;
    float y;
} Vector2;

typedef struct {
    float mass;
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    Vector2 force;
    SDL_Texture *texture;
} Object;

typedef struct {
    Object *objects[MAX_OBJECTS];
} GameObjects;

Object *object_create(char *path, SDL_Renderer *render, SDL_Window *window, float mass)
{
    Object *plnt = malloc(sizeof(Object));
    SDL_Surface *bmp = IMG_Load(path);
    if (!bmp) {
        perror("Cannot fasdfdsind bmp");
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
}

int update_physics(GameObjects *gs)
{
    Vector2 tmp;
    tmp.x = 0;
    tmp.y = 0;
    for(int i = 0; i < 3; i++) {
        gs->objects[i]->force = tmp;
    }

    for(int i = 0; i < 3; i++) {
        printf("\n");
        for(int j = i; j < 3; j++) {
            Object *plnt1 = gs->objects[i];
            Object *plnt2 = gs->objects[j];
            float x_diff = plnt1->pos.x - plnt2->pos.x;
            float y_diff = plnt1->pos.y - plnt2->pos.y;

            if(i !=j) {
                double force_x = C_BG * plnt1->mass * plnt2->mass / x_diff;
                float force_y = C_BG * plnt1->mass * plnt2->mass / y_diff;
                printf("Object %d and %d will feel x:%.15f y:%.15f N\n", i, j, force_x, force_y);
                gs->objects[0]->force.x += force_x;
                gs->objects[0]->force.y += force_y;
            }
        }
    }
    printf("Earth-> x: %.15f y:%.15f \n", gs->objects[0]->force.x, gs->objects[0]->force.y);

    for(int i = 0; i < 3; i++) {
        gs->objects[i]->force.y += 0.002;
        gs->objects[i]->force.x += 0.002;

        gs->objects[i]->acc.x = gs->objects[i]->force.x / gs->objects[i]->mass;
        gs->objects[i]->vel.x += gs->objects[i]->acc.x;
        gs->objects[i]->pos.x += gs->objects[i]->vel.x;

        gs->objects[i]->acc.y = gs->objects[i]->force.y / gs->objects[i]->mass;
        gs->objects[i]->vel.y += gs->objects[i]->acc.y;
        gs->objects[i]->pos.y += gs->objects[i]->vel.y;
    }

}

int main( int argc, char* args[] )
{
    GameObjects game_state;
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
        perror("Cannot init SDL");

    SDL_Window *window = SDL_CreateWindow("Hello World!", 100, 100, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if(!window)
        perror("Cannot open a window");

    SDL_Renderer *render = SDL_CreateRenderer(window, -1, 0);
    if(!render)
        perror("Cannot create a render");

    Object *earth = object_create("res/planet.bmp", render, window, 5);
    Object *mars = object_create("res/planet.bmp", render, window, 10);
    Object *venus = object_create("res/planet.bmp", render, window, 15);
    earth->pos.x = 0;
    earth->pos.y = 0;
    mars->pos.x = 10;
    mars->pos.y = 25;
    venus->pos.x = 25;
    venus->pos.y = 50;


    game_state.objects[0] = earth;
    game_state.objects[1] = mars;
    game_state.objects[2] = venus;

    bool quit = false;
    while(!quit) {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
            {
                quit = true;
                break;
            }
        }

        SDL_Rect e;
        e.x = earth->pos.x;
        e.y = earth->pos.y;
        e.w = 5;
        e.h = 5;

        SDL_Rect m;
        m.x = mars->pos.x;
        m.y = mars->pos.y;
        m.w =5;
        m.h =5;

        SDL_Rect v;
        v.x = venus->pos.x;
        v.y = venus->pos.y;
        v.w =5;
        v.h =5;



        //Main render
        update_physics(&game_state);

        SDL_RenderClear(render);
        SDL_RenderCopy(render, earth->texture, NULL, &e);
        SDL_RenderCopy(render, mars->texture, NULL, &m);
        SDL_RenderCopy(render, venus->texture, NULL, &v);
        SDL_RenderPresent(render);
    }

    object_destroy(earth);
    object_destroy(mars);
    object_destroy(venus);

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
