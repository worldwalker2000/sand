#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <raylib.h>

#define WINDOW_SIZE 800
#define FPS 60

enum ParticleType {
  AIR = 0,
  SAND,
  WATER,
  ROCK,
  BOMB,
  VOID
};
#define PARTICLE_TYPE_LAST VOID

Color particle_colors[] = {
  [AIR] = BLACK,
  [SAND] = YELLOW,
  [WATER] = BLUE,
  [ROCK] = GRAY,
  [BOMB] = DARKGRAY,
};

#define PARTICLE_SIZE 16
#define WORLD_SIZE (WINDOW_SIZE / PARTICLE_SIZE)
#define WORLD_SIZE_SQ (WORLD_SIZE * WORLD_SIZE)
struct World {
  enum ParticleType particles[WORLD_SIZE_SQ];
};

int
widx(int x, int y)
{
  return x + WORLD_SIZE * y;
}

#define RENDER_OFFSET ((WINDOW_SIZE % PARTICLE_SIZE) / 2)

int
ttop(int v)
{
  return RENDER_OFFSET + (v * PARTICLE_SIZE);
}

int
ptot(int v)
{
  return (int) floor((v - RENDER_OFFSET) / PARTICLE_SIZE);
}

void
particle_draw(int x, int y, struct World* world)
{
  enum ParticleType p = world->particles[widx(x, y)];
  if (p == VOID) {
    fprintf(stderr, "Void particle found in draw.\n");
    return;
  }

  DrawRectangle(ttop(x), ttop(y), PARTICLE_SIZE, PARTICLE_SIZE, particle_colors[p]);
}

enum Dir {
  N = 0,
  S,
  E,
  W,
  NW,
  SW,
  NE,
  SE,
};
#define DIR_LAST SE

void
move_in_dir(int* x, int* y, enum Dir dir)
{
  switch (dir) {
    case W:
      --*x;
      break;
    case E:
      ++*x;
      break;
    case N:
      --*y;
      break;
    case S:
      ++*y;
      break;
    case NE:
      ++*x;
      --*y;
      break;
    case SE:
      ++*x;
      ++*y;
      break;
    case NW:
      --*x;
      --*y;
      break;
    case SW:
      --*x;
      ++*y;
      break;
  }
}

void
swap_particles_in_dir(int x, int y, enum Dir dir, struct World* world)
{
  int dx = x, dy = y;
  move_in_dir(&dx, &dy, dir);

  enum ParticleType temp = world->particles[widx(x, y)];
  world->particles[widx(x, y)] = world->particles[widx(dx, dy)];
  world->particles[widx(dx, dy)] = temp;
}

void
set_particle_in_dir(int x, int y, enum Dir dir, enum ParticleType p, struct World* world)
{
  move_in_dir(&x, &y, dir);

  world->particles[widx(x, y)] = p;
}

enum ParticleType
get_particle_in_dir(int x, int y, enum Dir dir, struct World* world)
{
  // if (x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE) return VOID;

  move_in_dir(&x, &y, dir);

  if (x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE) return VOID;

  return world->particles[widx(x, y)];
}


void
particle_tick_noopp(int x, int y, struct World* world)
{
  (void) x;
  (void) y;
  (void) world;
}

void
particle_sand_tick(int x, int y, struct World* world)
{
  if (get_particle_in_dir(x, y, S, world) == AIR || get_particle_in_dir(x, y, S, world) == WATER) {
    swap_particles_in_dir(x, y, S, world);
  } else if (get_particle_in_dir(x, y, SW, world) == AIR || get_particle_in_dir(x, y, SW, world) == WATER) {
    swap_particles_in_dir(x, y, SW, world);
  } else if (get_particle_in_dir(x, y, SE, world) == AIR || get_particle_in_dir(x, y, SE, world) == WATER) {
    swap_particles_in_dir(x, y, SE, world);
  }
}

void
particle_water_tick(int x, int y, struct World* world)
{
  if (get_particle_in_dir(x, y, S, world) == AIR) {
    swap_particles_in_dir(x, y, S, world);
  }

  if (rand() % 10 < 5) {
    if (get_particle_in_dir(x, y, W, world) == AIR) {
      swap_particles_in_dir(x, y, W, world);
    } else if (get_particle_in_dir(x, y, SW, world) == AIR) {
      swap_particles_in_dir(x, y, SW, world);
    }
  } else {
    if (get_particle_in_dir(x, y, E, world) == AIR) {
      swap_particles_in_dir(x, y, E, world);
    } else if (get_particle_in_dir(x, y, SE, world) == AIR) {
      swap_particles_in_dir(x, y, SE, world);
    }
  }
}

void
particle_bomb_tick(int x, int y, struct World* world)
{
    const int bomb_size = 10;
    const int bomb_half_size = bomb_size / 2;
    for (int i = 0; i < DIR_LAST+1; ++i) {
        if (get_particle_in_dir(x, y, i, world) == WATER) {
            for (int rx = x - bomb_half_size; rx < x + bomb_half_size; ++rx) {
                for (int ry = y - bomb_half_size; ry < y + bomb_half_size; ++ry) {
                    world->particles[widx(rx, ry)] = AIR;
                }
            }
        }
    }
}

void (*particle_tick_functions[]) (int, int, struct World*) = {
  [AIR] = &particle_tick_noopp,
  [SAND] = &particle_sand_tick,
  [WATER] = &particle_water_tick,
  [ROCK] = &particle_tick_noopp,
  [BOMB] = &particle_bomb_tick,
};

void
particle_tick(int x, int y, struct World* world)
{
  enum ParticleType p = world->particles[widx(x, y)];
  if (p == VOID) {
    fprintf(stderr, "Void particle found in tick.\n");
    return;
  }

  particle_tick_functions[p](x, y, world);
}

int
main(void)
{
  srand(time(NULL));

  InitWindow(WINDOW_SIZE, WINDOW_SIZE, "sand");
  SetTargetFPS(FPS);

  struct World world;
  for (int i = 0; i < WORLD_SIZE_SQ; ++i) {
    // world.particles[i] = rand() % (PARTICLE_TYPE_LAST); // no +1 so that void particles are not spawned
    world.particles[i] = AIR;
  }

  int selected = 0;

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_ONE)) selected = 0;
    if (IsKeyPressed(KEY_TWO)) selected = 1;
    if (IsKeyPressed(KEY_THREE)) selected = 2;
    if (IsKeyPressed(KEY_FOUR)) selected = 3;
    if (IsKeyPressed(KEY_FIVE)) selected = 4;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      Vector2 mouse = GetMousePosition();
      world.particles[widx(ptot((int) mouse.x), ptot((int) mouse.y))] = selected;
    }


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
