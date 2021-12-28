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
  ROCK,
  VOID
} ParticleType;
#define PARTICLE_TYPE_LAST VOID

Color particle_colors[] = {
  [AIR] = BLACK,
  [SAND] = YELLOW,
  [WATER] = BLUE,
  [ROCK] = GRAY,
};

#define PARTICLE_SIZE 16
#define WORLD_SIZE (WINDOW_SIZE / PARTICLE_SIZE)
#define WORLD_SIZE_SQ (WORLD_SIZE * WORLD_SIZE)
#define WIDX(x, y) (x + WORLD_SIZE * y)
typedef struct {
  ParticleType particles[WORLD_SIZE_SQ];
} World;

#define RENDER_OFFSET ((WINDOW_SIZE % PARTICLE_SIZE) / 2)
#define TTOP(v) RENDER_OFFSET + (v * PARTICLE_SIZE)
#define PTOT(v) (int) floor((v - RENDER_OFFSET) / PARTICLE_SIZE)

void particle_draw(int x, int y, World* world) {
  ParticleType p = world->particles[WIDX(x, y)];
  DrawRectangle(TTOP(x), TTOP(y), PARTICLE_SIZE, PARTICLE_SIZE, particle_colors[p]);
}

typedef enum {
  N = 0,
  S,
  E,
  W,
  NW,
  SW,
  NE,
  SE,
} Dir;

void move_in_dir(int* x, int* y, Dir dir) {
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

void swap_particles_in_dir(int x, int y, Dir dir, World* world) {
  int dx = x, dy = y;
  move_in_dir(&dx, &dy, dir);

  ParticleType temp = world->particles[WIDX(x, y)];
  world->particles[WIDX(x, y)] = world->particles[WIDX(dx, dy)];
  world->particles[WIDX(dx, dy)] = temp;
}

ParticleType get_particle_in_dir(int x, int y, Dir dir, World* world) {
  // if (x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE) return VOID;

  move_in_dir(&x, &y, dir);

  if (x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE) return VOID;

  return world->particles[WIDX(x, y)];
}


void particle_air_tick(int x, int y, World* world) {
  (void) x;
  (void) y;
  (void) world;
}

void particle_sand_tick(int x, int y, World* world) {
  if (get_particle_in_dir(x, y, S, world) == AIR || get_particle_in_dir(x, y, S, world) == WATER) {
    swap_particles_in_dir(x, y, S, world);
  } else if (get_particle_in_dir(x, y, SW, world) == AIR || get_particle_in_dir(x, y, SW, world) == WATER) {
    swap_particles_in_dir(x, y, SW, world);
  } else if (get_particle_in_dir(x, y, SE, world) == AIR || get_particle_in_dir(x, y, SE, world) == WATER) {
    swap_particles_in_dir(x, y, SE, world);
  }
}

void particle_water_tick(int x, int y, World* world) {
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
  ParticleType p = world->particles[WIDX(x, y)];
  if (p == VOID) {
    fprintf(stderr, "Void particle found in tick.\n");
    return;
  }

  particle_tick_functions[p](x, y, world);
}

int main(void)
{
  srand(time(NULL));

  InitWindow(WINDOW_SIZE, WINDOW_SIZE, "sand");
  SetTargetFPS(FPS);

  World world;
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

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      Vector2 mouse = GetMousePosition();
      world.particles[WIDX(PTOT((int) mouse.x), PTOT((int) mouse.y))] = selected;
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
