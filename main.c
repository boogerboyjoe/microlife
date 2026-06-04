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
#define MAX_MEAT_AGE 5

#define TOTAL_CHUNKS (CHUNK_NUMBER_X * CHUNK_NUMBER_Y)
#define TILES_PER_CHUNK (CHUNK_SIZE * CHUNK_SIZE)
#define WORLD_WIDTH  (CHUNK_NUMBER_X * CHUNK_SIZE)
#define WORLD_HEIGHT (CHUNK_NUMBER_Y * CHUNK_SIZE)
#define TOTAL_TILES  (WORLD_WIDTH * WORLD_HEIGHT)

bool debug = false;

typedef struct {
	unsigned char type;
	int age;
	unsigned char energy;
}next_tile;

typedef struct {
	unsigned char type;
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

Image world_image;
Color* world_pixels;
Texture2D world_texture;

int ui_sizing = 0;

void initialize_world(void) {
	game_world.chunks = malloc(TOTAL_CHUNKS * sizeof(chunk));
	game_world.tiles = malloc(TOTAL_TILES * sizeof(tile));
	game_world.next_tiles = malloc(TOTAL_TILES * sizeof(next_tile));
	for (int i = 0; i < TOTAL_TILES; i++) {
		game_world.tiles[i].type = 0;
		game_world.tiles[i].age = 0;
		game_world.tiles[i].energy = 0;
		if (i == (WORLD_HEIGHT/2) * WORLD_WIDTH + (WORLD_WIDTH/2)) {
			game_world.tiles[i].type = 2;
		}
		game_world.next_tiles[i].type = 0;
		game_world.next_tiles[i].age = 0;
		game_world.next_tiles[i].energy = 0;
	}
}

void uninitialize_world(void) {
	free(game_world.next_tiles);
	free(game_world.tiles);
	free(game_world.chunks);
}

bool is_tile(unsigned char* types, int len, int pos) {
	bool exists = false;
	for (int i = 0; i < len; i++) {
		if (game_world.tiles[pos].type == types[i] && (game_world.next_tiles[pos].type == types[i] || game_world.next_tiles[pos].type == 0)) {
			exists = true;
		}
	}
	return exists;
}

int move_and_dupe(int new_tile_index, int tile_index, unsigned char type, int age, int energy, unsigned char energy_needed, unsigned char reproduce_type) {
	if (type == 2) {
		// IF PLANT
		game_world.next_tiles[tile_index].type = type;
		game_world.next_tiles[tile_index].age = age + 1;
		energy++;
		if (energy >= energy_needed) {
			// REPRODUCE
			game_world.next_tiles[new_tile_index].type = reproduce_type;
			game_world.next_tiles[new_tile_index].age = 0;
			game_world.next_tiles[new_tile_index].energy = 0;
			energy = 0;
		}
		game_world.next_tiles[tile_index].energy = energy;
	}
	else {
		// IF NOT PLANT
		game_world.next_tiles[new_tile_index].type = type;
		game_world.next_tiles[new_tile_index].age = age;
		energy++;
		if (energy >= energy_needed) {
			// REPRODUCE
			game_world.next_tiles[tile_index].type = reproduce_type;
			game_world.next_tiles[tile_index].age = 0;
			game_world.next_tiles[tile_index].energy = 0;
			energy = 0;
		}
		game_world.next_tiles[new_tile_index].energy = energy;
	}
	return energy;
}

void update_world(void) {
	for (int y = 0; y < WORLD_HEIGHT; y++) {
		for (int x = 0; x < WORLD_WIDTH; x++) {
			int tile_index = y * (WORLD_WIDTH)+x;
			unsigned char type = game_world.tiles[tile_index].type;
			if (type == 0) continue;
			if (game_world.next_tiles[tile_index].type != 0) continue;
			int age = game_world.tiles[tile_index].age;
			int energy = game_world.tiles[tile_index].energy;
			int max_age = 0;
			bool has_moved = false;
			// 0 = none, 1 = testcell, 2 = plantcell, 3 = deadplantcell, 4 = herbavore, 5 = carnavore, 6 = decomposer
			switch (type) {
				case 2:
					max_age = MAX_PLANT_AGE * 4;
					break;
				case 4:
					max_age = MAX_MEAT_AGE * 12;
					break;
				case 5:
					max_age = MAX_MEAT_AGE * 48;
					break;
				case 6:
					max_age = MAX_MEAT_AGE * 3;
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

								if (is_tile((unsigned char[]) { 0 }, 1, new_tile_index)) {
									unsigned int mutation_chance = rand() % 8192;
									if (mutation_chance == 0) {
										energy = move_and_dupe(new_tile_index, tile_index, type, age, energy, 2, 6);
									}
									else {
										energy = move_and_dupe(new_tile_index, tile_index, type, age, energy, 2, type);
									}
									break;
								}
							}
						}
						game_world.next_tiles[tile_index].type = type;
						game_world.next_tiles[tile_index].age = age + 1;
						break;
					case 3:
						if (!is_tile((unsigned char[]) { 0 }, 1, tile_index)) {
							game_world.next_tiles[tile_index].type = type;
							game_world.next_tiles[tile_index].age = 0;
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

								if (is_tile((unsigned char[]) { 0 }, 1, new_tile_index)) {
									has_moved = true;
									game_world.next_tiles[new_tile_index].type = type;
									game_world.next_tiles[new_tile_index].age = age + 1;
									game_world.next_tiles[new_tile_index].energy = energy;
									break;
								} else if (is_tile((unsigned char[]) { 2 }, 1, new_tile_index)) {
									has_moved = true;
									unsigned int mutation_chance = rand() % 8192;
									if (mutation_chance == 0) {
										energy = move_and_dupe(new_tile_index, tile_index, type, age, energy, 2, 5);
									}
									else {
										energy = move_and_dupe(new_tile_index, tile_index, type, age, energy, 2, type);
									}
									break;
								}
							}
						}
						if (has_moved == false) {
							game_world.next_tiles[tile_index].type = type;
							game_world.next_tiles[tile_index].age = age + 1;
							game_world.next_tiles[tile_index].energy = energy;
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

								if (is_tile((unsigned char[]) { 0, 2, 3 }, 3, new_tile_index)) {
									has_moved = true;
									game_world.next_tiles[new_tile_index].type = type;
									game_world.next_tiles[new_tile_index].age = age + 1;
									break;
								}
								else if (is_tile((unsigned char[]) { 4, 6 }, 2, new_tile_index)) {
									has_moved = true;
									energy = move_and_dupe(new_tile_index, tile_index, type, age, energy, 2, type);
									break;
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
								if (is_tile((unsigned char[]) { 0 }, 1, new_tile_index)) {
									has_moved = true;
									game_world.next_tiles[new_tile_index].type = type;
									game_world.next_tiles[new_tile_index].age = age + 1;
									game_world.next_tiles[new_tile_index].energy = energy;
									break;
								}
								else if (is_tile((unsigned char[]) { 3 }, 1, new_tile_index)) {
									has_moved = true;
									unsigned int mutation_chance = rand() % 8192;
									if (mutation_chance == 0) {
										energy = move_and_dupe(new_tile_index, tile_index, type, age, energy, 2, 4);
									}
									else {
										energy = move_and_dupe(new_tile_index, tile_index, type, age, energy, 2, type);
									}
									break;
								}
							}
						}
						if (has_moved == false) {
							game_world.next_tiles[tile_index].type = type;
							game_world.next_tiles[tile_index].age = age + 1;
							game_world.next_tiles[tile_index].energy = energy;
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
				game_world.next_tiles[tile_index].age = 0;
			}
		}
	}
	for (int i = 0; i < TOTAL_TILES; i++) {
		game_world.tiles[i].type = game_world.next_tiles[i].type;
		game_world.tiles[i].age = game_world.next_tiles[i].age;
		game_world.tiles[i].energy = game_world.next_tiles[i].energy;
		game_world.next_tiles[i].type = 0;
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

void draw_ui(int current_screen_width, int current_screen_height, double fps) {
	ui_sizing = current_screen_height * 0.1;
	char* fps_text = TextFormat("%.1lf fps selected", fps);
	DrawText(fps_text, current_screen_width * 0.05, current_screen_height - ui_sizing, 0.4 * ui_sizing, LIGHTGRAY);
	DrawText("Q and E to change simulation FPS", current_screen_width * 0.05, current_screen_height - 0.45 * ui_sizing, 0.4 * ui_sizing, GRAY);
}

int main(void) {
	double time_passed = 0;

	initialize_world();

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 600, "MicroLife");

	int monitor = GetCurrentMonitor();

	int screen_width = GetMonitorWidth(monitor);
	int screen_height = GetMonitorHeight(monitor);

	int current_screen_width = GetScreenWidth();
	int current_screen_height = GetScreenHeight();

	world_image = GenImageColor(WORLD_WIDTH, WORLD_HEIGHT, BLACK);

	world_texture = LoadTextureFromImage(world_image);

	world_pixels = (Color*)world_image.data;

	SetTextureFilter(world_texture, TEXTURE_FILTER_POINT);

	SetTargetFPS(240);

	double fps = 60.0;

	srand(time(NULL));

	while (!WindowShouldClose()) {
		if (IsWindowResized()) {
			current_screen_width = GetScreenWidth();
			current_screen_height = GetScreenHeight();
		}

		BeginDrawing();
		time_passed += GetFrameTime();
		if (time_passed > 1.0 / fps) {
			time_passed -= 1.0 / fps;
			update_world();
		}

		ClearBackground((Color) { 25, 25, 50, 255 });
		draw_screen();
		UpdateTexture(world_texture, world_pixels);
		DrawTexturePro(world_texture, (Rectangle) { 0, 0, WORLD_WIDTH, WORLD_HEIGHT }, (Rectangle) { 0, 0, WORLD_WIDTH * TILE_SIZE, WORLD_HEIGHT * TILE_SIZE}, (Vector2) { 0, 0 }, 0.0f, WHITE);

		if (IsKeyPressed(KEY_Q) && fps > 10.0) {
			fps -= 10.0;
		}
		else if (IsKeyPressed(KEY_Q) && fps > 1.0) {
			fps -= 1.0;
		}
		if (IsKeyPressed(KEY_E) && fps > 9.0) {
			fps += 10.0;
		}
		else if (IsKeyPressed(KEY_E)) {
			fps += 1.0;
		}

		draw_ui(current_screen_width, current_screen_height, fps);
		if (debug == true) {
			printf("%f Seconds Elapsed Per Frame\n", GetFrameTime());
		}
		EndDrawing();
	}

	UnloadTexture(world_texture);
	UnloadImage(world_image);

	uninitialize_world();
	
	CloseWindow();
	return 0;
}