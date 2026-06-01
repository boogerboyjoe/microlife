#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define TILE_SIZE 4
#define CHUNK_SIZE 16
#define CHUNK_NUMBER_X 16
#define CHUNK_NUMBER_Y 16
#define MAX_ORGANISMS 10000
#define MAX_TILES_PER_ORGANISM 1000
#define MAX_PLANT_AGE 1
#define MAX_MEAT_AGE 4

#define TOTAL_CHUNKS (CHUNK_NUMBER_X * CHUNK_NUMBER_Y)
#define TILES_PER_CHUNK (CHUNK_SIZE * CHUNK_SIZE)
#define WORLD_WIDTH  (CHUNK_NUMBER_X * CHUNK_SIZE)
#define WORLD_HEIGHT (CHUNK_NUMBER_Y * CHUNK_SIZE)
#define TOTAL_TILES  (WORLD_WIDTH * WORLD_HEIGHT)

//typedef struct {
	//unsigned short energy;
	//unsigned short tile_count;
	//unsigned char mutation_rate;
	//bool alive;
//}organism;

typedef struct {
	unsigned char type;
	//unsigned int id;
	int age;
	unsigned char energy;
}next_tile;

typedef struct {
	unsigned char type;
	//unsigned int id;
	int age;
	unsigned char energy;
}tile;

typedef struct {
	bool active;
}chunk;

typedef struct {
	tile* tiles;
	next_tile* next_tiles;
	chunk* chunks;
}world;

world game_world;
//organism organisms[MAX_ORGANISMS];

Image world_image;
Color* world_pixels;
Texture2D world_texture;

void initialize_world(void) {
	game_world.chunks = malloc(TOTAL_CHUNKS * sizeof(chunk));
	game_world.tiles = malloc(TOTAL_TILES * sizeof(tile));
	game_world.next_tiles = malloc(TOTAL_TILES * sizeof(next_tile));
	for (int i = 0; i < TOTAL_TILES; i++) {
		game_world.tiles[i].type = 0;
		//game_world.tiles[i].id = 0;
		game_world.tiles[i].age = 0;
		game_world.tiles[i].energy = 0;
		if (i == (WORLD_HEIGHT/2) * WORLD_WIDTH + (WORLD_WIDTH/2)) {
			game_world.tiles[i].type = 2;
			//game_world.tiles[i].id = 0;
		}
		game_world.next_tiles[i].type = 0;
		//game_world.next_tiles[i].id = 0;
		game_world.next_tiles[i].age = 0;
		game_world.next_tiles[i].energy = 0;
	}
}

void uninitialize_world(void) {
	free(game_world.next_tiles);
	free(game_world.tiles);
	free(game_world.chunks);
}

void update_world(void) {
	for (int y = 0; y < WORLD_HEIGHT; y++) {
		for (int x = 0; x < WORLD_WIDTH; x++) {
			int tile_index = y * (WORLD_WIDTH)+x;
			unsigned char type = game_world.tiles[tile_index].type;
			//unsigned int id = game_world.tiles[tile_index].id;
			int age = game_world.tiles[tile_index].age;
			int energy = game_world.tiles[tile_index].energy;
			int max_age = 0;
			bool has_moved = false;
			switch (type) {
				case 2:
					max_age = MAX_PLANT_AGE;
					break;
				case 4:
					max_age = MAX_MEAT_AGE * 6;
					break;
				case 5:
					max_age = MAX_MEAT_AGE * 4;
					break;
				case 6:
					max_age = MAX_MEAT_AGE;
					break;
			}
			if (age <= max_age) {
				switch (type) {
					case 2:
						for (int i = 0; i < 2; i++) {
							unsigned char expand_direction = rand() % 4;
							// 0 = up, 1 = right, 2 = down, 3 = left

							int new_y = y;
							int new_x = x;

							switch (expand_direction) {
							case 0:
								new_y = y - 1;
								break;
							case 1:
								new_x = x + 1;
								break;
							case 2:
								new_y = y + 1;
								break;
							default:
								new_x = x - 1;
							}
							if (new_x >= 0 && new_x < WORLD_WIDTH && new_y >= 0 && new_y < WORLD_HEIGHT) {
								int new_tile_index = new_y * (WORLD_WIDTH)+new_x;
								unsigned char next_type = game_world.tiles[new_tile_index].type;

								if (next_type == 0 && game_world.next_tiles[new_tile_index].type == 0) {
									unsigned int mutation_chance = rand() % 4096;
									if (mutation_chance == 0) {
										game_world.next_tiles[new_tile_index].type = 6;
									}
									else {
										game_world.next_tiles[new_tile_index].type = type;
									}
									//game_world.next_tiles[new_tile_index].id = id;
									game_world.next_tiles[new_tile_index].age = 0;
									i = 3;
								}
							}
						}
						game_world.next_tiles[tile_index].type = type;
						//game_world.next_tiles[tile_index].id = id;
						game_world.next_tiles[tile_index].age = age + 1;
						break;
					case 3:
						if (game_world.next_tiles[tile_index].type == 0) {
							game_world.next_tiles[tile_index].type = type;
							//game_world.next_tiles[tile_index].id = id;
							game_world.next_tiles[tile_index].age = age;
						}
						break;
					case 4:
						for (int i = 0; i < 3; i++) {
							unsigned char expand_direction = rand() % 4;
							// 0 = up, 1 = right, 2 = down, 3 = left

							int new_y = y;
							int new_x = x;

							switch (expand_direction) {
							case 0:
								new_y = y - 1;
								break;
							case 1:
								new_x = x + 1;
								break;
							case 2:
								new_y = y + 1;
								break;
							default:
								new_x = x - 1;
							}
							if (new_x >= 0 && new_x < WORLD_WIDTH && new_y >= 0 && new_y < WORLD_HEIGHT) {
								int new_tile_index = new_y * (WORLD_WIDTH)+new_x;
								unsigned char next_type = game_world.tiles[new_tile_index].type;

								if (next_type == 0 && game_world.next_tiles[new_tile_index].type == 0) {
									has_moved = true;
									game_world.next_tiles[new_tile_index].type = type;
									//game_world.next_tiles[new_tile_index].id = id;
									game_world.next_tiles[new_tile_index].age = age + 1;
									i = 3;
								} else if (next_type == 2 || next_type == 3) {
									has_moved = true;
									unsigned int mutation_chance = rand() % 4096;
									if (mutation_chance == 0) {
										game_world.next_tiles[new_tile_index].type = 5;
									}
									else {
										game_world.next_tiles[new_tile_index].type = type;
									}
									//game_world.next_tiles[new_tile_index].id = id;
									game_world.next_tiles[new_tile_index].age = age + 1;
									energy++;
									if (energy >= 2) {
										game_world.next_tiles[tile_index].type = type;
										//game_world.next_tiles[tile_index].id = id;
										game_world.next_tiles[tile_index].age = 0;
										game_world.next_tiles[tile_index].energy = 0;
										energy = 0;
									}
									game_world.next_tiles[new_tile_index].energy = energy;
									i = 3;
								}
							}
						}
						if (has_moved == false) {
							game_world.next_tiles[tile_index].type = type;
						}
						break;
					case 5:
						for (int i = 0; i < 3; i++) {
							unsigned char expand_direction = rand() % 4;
							// 0 = up, 1 = right, 2 = down, 3 = left

							int new_y = y;
							int new_x = x;

							switch (expand_direction) {
							case 0:
								new_y = y - 1;
								break;
							case 1:
								new_x = x + 1;
								break;
							case 2:
								new_y = y + 1;
								break;
							default:
								new_x = x - 1;
							}
							if (new_x >= 0 && new_x < WORLD_WIDTH && new_y >= 0 && new_y < WORLD_HEIGHT) {
								int new_tile_index = new_y * (WORLD_WIDTH)+new_x;
								unsigned char next_type = game_world.tiles[new_tile_index].type;

								if (next_type == 0 && game_world.next_tiles[new_tile_index].type == 0) {
									has_moved = true;
									game_world.next_tiles[new_tile_index].type = type;
									//game_world.next_tiles[new_tile_index].id = id;
									game_world.next_tiles[new_tile_index].age = age + 1;
									i = 3;
								}
								else if (next_type == 4 || next_type == 6) {
									has_moved = true;
									game_world.next_tiles[tile_index].type = type;
									//game_world.next_tiles[tile_index].id = id;
									game_world.next_tiles[tile_index].age = 0;
									game_world.next_tiles[new_tile_index].type = type;
									//game_world.next_tiles[new_tile_index].id = id;
									game_world.next_tiles[new_tile_index].age = age + 1;
									i = 3;
								}
							}
						}
						if (has_moved == false) {
							game_world.next_tiles[tile_index].type = type;
							game_world.next_tiles[tile_index].age = age + 1;
							game_world.next_tiles[tile_index].energy = energy;
						}
						break;
					case 6:
						for (int i = 0; i < 3; i++) {
							unsigned char expand_direction = rand() % 4;
							// 0 = up, 1 = right, 2 = down, 3 = left

							int new_y = y;
							int new_x = x;

							switch (expand_direction) {
							case 0:
								new_y = y - 1;
								break;
							case 1:
								new_x = x + 1;
								break;
							case 2:
								new_y = y + 1;
								break;
							default:
								new_x = x - 1;
							}
							if (new_x >= 0 && new_x < WORLD_WIDTH && new_y >= 0 && new_y < WORLD_HEIGHT) {
								int new_tile_index = new_y * (WORLD_WIDTH)+new_x;
								unsigned char next_type = game_world.tiles[new_tile_index].type;

								if (next_type == 0 && game_world.next_tiles[new_tile_index].type == 0) {
									has_moved = true;
									game_world.next_tiles[new_tile_index].type = type;
									//game_world.next_tiles[new_tile_index].id = id;
									game_world.next_tiles[new_tile_index].age = age + 1;
									i = 3;
								}
								else if (next_type == 3) {
									has_moved = true;
									unsigned int mutation_chance = rand() % 4096;
									if (mutation_chance == 0) {
										game_world.next_tiles[tile_index].type = 4;
									}
									else {
										game_world.next_tiles[tile_index].type = type;
									}
									//game_world.next_tiles[tile_index].id = id;
									game_world.next_tiles[tile_index].age = 0;
									game_world.next_tiles[new_tile_index].type = type;
									//game_world.next_tiles[new_tile_index].id = id;
									game_world.next_tiles[new_tile_index].age = age + 1;
									i = 3;
								}
							}
						}
						if (has_moved == false) {
							game_world.next_tiles[tile_index].type = type;
						}
						break;

				}
			} else {
				if (type == 2) {
					unsigned char corpse_chance = rand() % 2;
					if (corpse_chance != 0) {
						game_world.next_tiles[tile_index].type = 3;
					}
				} else {
					game_world.next_tiles[tile_index].type = 0;
				}
				//game_world.next_tiles[tile_index].id = 0;
				game_world.next_tiles[tile_index].age = 0;
			}
		}
	}
	for (int i = 0; i < TOTAL_TILES; i++) {
		game_world.tiles[i].type = game_world.next_tiles[i].type;
		//game_world.tiles[i].id = game_world.next_tiles[i].id;
		game_world.tiles[i].age = game_world.next_tiles[i].age;
		game_world.tiles[i].energy = game_world.next_tiles[i].energy;
		game_world.next_tiles[i].type = 0;
		//game_world.next_tiles[i].id = 0;
		game_world.next_tiles[i].age = 0;
		game_world.next_tiles[i].energy = 0;
	}
}

void draw_screen(void) {
	Color tile_color = (Color){ 0, 0, 0, 0 };

	for (int y = 0; y < WORLD_HEIGHT; y++) {
		for (int x = 0; x < WORLD_WIDTH; x++) {
			int tile_index = y * (WORLD_WIDTH)+x;
			unsigned char type = game_world.tiles[tile_index].type;
			// 0 = none, 1 = testcell, 2 = plantcell, 3 = deadplantcell, 4 = herbavore, 5 = carnavore, 6 = decomposer

			switch (type) {
			case 0:
				world_pixels[tile_index] = BLANK;
				break;
			case 1:
				tile_color = (Color){ (unsigned char)(((float)x / WORLD_WIDTH) * 255), (unsigned char)(((float)y / WORLD_HEIGHT) * 255), 0, 255 };

				world_pixels[tile_index] = tile_color;
				break;
			case 2:
				tile_color = (Color){ 60, 170, 5, 255 };

				world_pixels[tile_index] = tile_color;
				break;
			case 3:
				tile_color = (Color){ 60, 70, 5, 255 };

				world_pixels[tile_index] = tile_color;
				break;
			case 4:
				tile_color = (Color){ 60, 150, 80, 255 };

				world_pixels[tile_index] = tile_color;
				break;
			case 5:
				tile_color = (Color){ 160, 60, 5, 255 };

				world_pixels[tile_index] = tile_color;
				break;
			case 6:
				tile_color = (Color){ 140, 120, 5, 255 };

				world_pixels[tile_index] = tile_color;
				break;
			}
		}
	}
}

int main(void) {
	clock_t start_time, stop_time;
	double ms_elapsed;

	initialize_world();

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1, 1, "MicroLife");

	int monitor = GetCurrentMonitor();

	int screen_width = GetMonitorWidth(monitor);
	int screen_height = GetMonitorHeight(monitor);

	SetWindowSize(screen_width / 2, screen_height / 2);

	world_image = GenImageColor(WORLD_WIDTH, WORLD_HEIGHT, BLACK);

	world_texture = LoadTextureFromImage(world_image);

	world_pixels = (Color*)world_image.data;

	SetTextureFilter(world_texture, TEXTURE_FILTER_POINT);

	SetTargetFPS(60);
	srand(time(NULL));

	while (!WindowShouldClose()) {
		start_time = clock();

		update_world();

		BeginDrawing();

		ClearBackground((Color) { 25, 25, 50, 255 });
		draw_screen();

		UpdateTexture(world_texture, world_pixels);

		DrawTexturePro(world_texture, (Rectangle) { 0, 0, WORLD_WIDTH, WORLD_HEIGHT }, (Rectangle) { 0, 0, WORLD_WIDTH * TILE_SIZE, WORLD_HEIGHT * TILE_SIZE}, (Vector2) { 0, 0 }, 0.0f, WHITE);

		stop_time = clock();
		ms_elapsed = ((stop_time - start_time) * 1000.0 / CLOCKS_PER_SEC);
		printf("%f MS Elapsed Per Frame\n", ms_elapsed);
		EndDrawing();
	}

	UnloadTexture(world_texture);
	UnloadImage(world_image);

	uninitialize_world();
	
	CloseWindow();
	return 0;
}