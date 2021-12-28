#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <raylib.h>

#define WINDOW_SIZE 800
#define FPS 60

typedef enum {
  AIR = 0,
  SAND,
  WATER,
  ROCK
} ParticleType;
#define PARTICLE_TYPE_LAST ROCK

Color particle_colors[] = {
  [AIR] = BLACK,
  [SAND] = YELLOW,
  [WATER] = BLUE,
  [ROCK] = GRAY,
};

#define PARTICLE_SIZE 64
#define WORLD_SIZE (WINDOW_SIZE / PARTICLE_SIZE)
#define WORLD_SIZE_SQ (WORLD_SIZE * WORLD_SIZE)
typedef struct {
  ParticleType particles[WORLD_SIZE_SQ];
} World;

#define RENDER_OFFSET ((WINDOW_SIZE % PARTICLE_SIZE) / 2)
#define TTOP(v) RENDER_OFFSET + (v * PARTICLE_SIZE)
#define PTOT(v) floor((v - RENDER_OFFSET) / PARTICLE_SIZE)

void particle_draw(int x, int y, World* world) {
  ParticleType p = world->particles[x + WORLD_SIZE * y];
  DrawRectangle(TTOP(x), TTOP(y), PARTICLE_SIZE, PARTICLE_SIZE, particle_colors[p]);
}

void particle_air_tick(int x, int y, World* world) {
  (void) x;
  (void) y;
  (void) world;
}

void particle_sand_tick(int x, int y, World* world) {
  (void) x;
  (void) y;
  (void) world;
}

void particle_water_tick(int x, int y, World* world) {
  (void) x;
  (void) y;
  (void) world;
}

void particle_rock_tick(int x, int y, World* world) {
  (void) x;
  (void) y;
  (void) world;
}

void (*particle_tick_functions[]) (int, int, World*) = {
  [AIR] = &particle_air_tick,
  [SAND] = &particle_sand_tick,
  [WATER] = &particle_water_tick,
  [ROCK] = &particle_rock_tick
};

void particle_tick(int x, int y, World* world) {
  ParticleType p = world->particles[x + WORLD_SIZE * y];
  particle_tick_functions[p](x, y, world);
}

int main(void)
{
  srand(time(NULL));

  InitWindow(WINDOW_SIZE, WINDOW_SIZE, "sand");
  SetTargetFPS(FPS);

  World world;
  for (int i = 0; i < WORLD_SIZE_SQ; ++i) {
    world.particles[i] = rand() % (PARTICLE_TYPE_LAST + 1);
  }

  while (!WindowShouldClose()) {
    for (int x = 0; x < WORLD_SIZE; ++x)
      for (int y = 0; y < WORLD_SIZE; ++y)
        particle_tick(x, y, &world);

    BeginDrawing();
    ClearBackground(LIGHTGRAY);

    for (int x = 0; x < WORLD_SIZE; ++x)
      for (int y = 0; y < WORLD_SIZE; ++y)
        particle_draw(x, y, &world);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
