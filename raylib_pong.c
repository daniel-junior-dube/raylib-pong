#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define PROJECT_NAME "raylib-pong"
#define TARGET_FPS 60
#define PANEL_MARGIN 32
#define H1_FONT_SIZE 10
#define BALL_SPEED 10.0f

struct Dimensions {
	int screen_width;
	int screen_height;
	int paddle_width;
	int paddle_height;
	int ball_radius;
};

enum ScreenResolution {
	R256X144,
	R512X288,
	R768X432,
};

enum Screen {
	MAIN_MENU_SCREEN,
	IN_GAME_SCREEN
};

enum MenuOption {
	MENU_OPTION_START,
	MENU_OPTION_QUIT,
};

static const char *MAIN_MENU_OPTIONS_STRING[] = {
	"Start", "Quit"
};

struct GlobalState {
	struct Dimensions dimensions;
	enum Screen current_screen;
};

struct MainMenuState {
	enum MenuOption selected_option;
};

struct InGameState {
	int player_1_score;
	int player_2_score;
	Vector2 ball_pos;
	Vector2 ball_dir;
	float ball_speed;
	Rectangle left_paddle;
	Rectangle right_paddle;
};

static const Vector2 UP_NORMAL = { 0.0f, 1.0f };
static const Vector2 DOWN_NORMAL = { 0.0f, -1.0f };
static const Vector2 RIGHT_NORMAL = { 1.0f, 0.0f };
static const Vector2 LEFT_NORMAL = { -1.0f, 0.0f };

void update_main_menu_screen(struct GlobalState* global_state, struct MainMenuState* state) {

	// ? Processing inputs
	if (IsKeyPressed(KEY_UP)) {
		--state->selected_option;
		if (state->selected_option == -1) {
			state->selected_option = MENU_OPTION_QUIT;
		}
	}
	if (IsKeyPressed(KEY_DOWN)) {
		state->selected_option = (state->selected_option + 1) % (MENU_OPTION_QUIT + 1);
	}
	if (IsKeyPressed(KEY_SPACE)) {
		switch (state->selected_option) {
			case MENU_OPTION_START:
				global_state->current_screen = IN_GAME_SCREEN;
				break;
			case MENU_OPTION_QUIT:
				CloseWindow();
				break;
			default:
				break;
		}
	}

	// ? Updating
	// ...

	// ? Rendering
	BeginDrawing();
	ClearBackground(RAYWHITE);
	DrawText("PONG", PANEL_MARGIN, PANEL_MARGIN, H1_FONT_SIZE, BLACK);
	for (short i = MENU_OPTION_START; i <= MENU_OPTION_QUIT; ++i) {
		Color option_color =
			i == state->selected_option ? RED : LIGHTGRAY;
		DrawText(
			MAIN_MENU_OPTIONS_STRING[i],
			PANEL_MARGIN,
			PANEL_MARGIN + H1_FONT_SIZE + 8 + (i * (PANEL_MARGIN + 8)),
			PANEL_MARGIN, option_color
		);
	}
	EndDrawing();
}

struct Vector2 Vector2Reflect(struct Vector2 v, struct Vector2 l) {
	float dotProduct = Vector2DotProduct(v, l);
	return Vector2Subtract(v, Vector2Scale(l, 2.0f * dotProduct));
}

void update_in_game_screen(struct GlobalState* global_state, struct InGameState* state) {
	int screen_width = global_state->dimensions.screen_width;
	int screen_height = global_state->dimensions.screen_height;
	int paddle_height = global_state->dimensions.paddle_height;
	int paddle_width = global_state->dimensions.paddle_width;
	int ball_radius = global_state->dimensions.ball_radius;

	// ? Processing inputs
	// ...
	if (IsKeyDown(KEY_W)) {
		state->left_paddle.y -= BALL_SPEED / 2.0f;
		if (state->left_paddle.y < 0.0f) {
			state->left_paddle.y = 0.0f;
		}
	}
	if (IsKeyDown(KEY_S)) {
		state->left_paddle.y += BALL_SPEED / 2.0f;
		float max_paddle_y = screen_height - paddle_height;
		if (state->left_paddle.y > max_paddle_y) {
			state->left_paddle.y = max_paddle_y;
		}
	}
	if (IsKeyDown(KEY_UP)) {
		state->right_paddle.y -= BALL_SPEED / 2.0f;
		if (state->right_paddle.y < 0.0f) {
			state->right_paddle.y = 0.0f;
		}
	}
	if (IsKeyDown(KEY_DOWN)) {
		state->right_paddle.y += BALL_SPEED / 2.0f;
		float max_paddle_y = screen_height - paddle_height;
		if (state->right_paddle.y > max_paddle_y) {
			state->right_paddle.y = max_paddle_y;
		}
	}

	// ? Updating

	Vector2 ball_vec = Vector2Scale(state->ball_dir, state->ball_speed);

	// Wall bounce
	state->ball_pos = Vector2Add(state->ball_pos, ball_vec);
	if ((int)state->ball_pos.y >= screen_height - ball_radius) {
		state->ball_dir = Vector2Reflect(state->ball_dir, UP_NORMAL);
	}
	if ((int)state->ball_pos.y <= ball_radius) {
		state->ball_dir = Vector2Reflect(state->ball_dir, DOWN_NORMAL);
	}

	// Point score
	if ((int)state->ball_pos.x >= screen_width) {
		++state->player_1_score;
		state->ball_pos.x = (float)screen_width / 2.0f;
		state->ball_pos.y = (float)screen_height / 2.0f;
	}
	if ((int)state->ball_pos.x < 0) {
		++state->player_2_score;
		state->ball_pos.x = (float)screen_width / 2.0f;
		state->ball_pos.y = (float)screen_height / 2.0f;
	}

	// Paddle collision (reflect vel on paddle's normal)
	if (ball_vec.x < 0.0f && CheckCollisionCircleRec(state->ball_pos, ball_radius, state->left_paddle)) {
		
		// above
		if (state->ball_pos.y < state->left_paddle.y) {

			// top-right
			if (state->ball_pos.x > state->left_paddle.x + paddle_width) {
				Vector2 top_right_corner = {
					state->left_paddle.x + paddle_width,
					state->left_paddle.y
				};
				state->ball_dir = Vector2Normalize(Vector2Subtract(state->ball_pos, top_right_corner));
				state->ball_pos = Vector2Add(top_right_corner, Vector2Scale(state->ball_dir, ball_radius));

			// top
			} else {

				// Reflect up vector
				state->ball_dir = Vector2Reflect(state->ball_dir, UP_NORMAL);
			}

		// below
		} else if (state->ball_pos.y > state->left_paddle.y + paddle_height) {

			// bottom-right
			if (state->ball_pos.x > state->left_paddle.x + paddle_width) {

				// Check normalized vec between ball center and bottom-right corner
				Vector2 bottom_right_corner = {
					state->left_paddle.x + paddle_width,
					state->left_paddle.y + paddle_height
				};
				state->ball_dir = Vector2Normalize(Vector2Subtract(state->ball_pos, bottom_right_corner));
				state->ball_pos = Vector2Add(bottom_right_corner, Vector2Scale(state->ball_dir, ball_radius));
			
			// bottom
			} else {

				// Reflect down vector
				state->ball_dir = Vector2Reflect(state->ball_dir, DOWN_NORMAL);
			}

		// right
		} else {
			state->ball_dir = Vector2Reflect(state->ball_dir, RIGHT_NORMAL);
		}
	}
	if (ball_vec.x > 0.0f && CheckCollisionCircleRec(state->ball_pos, ball_radius, state->right_paddle)) {
		

		// above
		if (state->ball_pos.y < state->right_paddle.y) {

			// top-left
			if (state->ball_pos.x < state->right_paddle.x) {
				Vector2 top_left_corner = {
					state->right_paddle.x,
					state->right_paddle.y
				};
				state->ball_dir = Vector2Normalize(Vector2Subtract(state->ball_pos, top_left_corner));
				state->ball_pos = Vector2Add(top_left_corner, Vector2Scale(state->ball_dir, ball_radius));

			// top
			} else {

				// Reflect up vector
				state->ball_dir = Vector2Reflect(state->ball_dir, UP_NORMAL);
			}

		// below
		} else if (state->ball_pos.y > state->right_paddle.y + paddle_height) {

			// bottom-left
			if (state->ball_pos.x < state->right_paddle.x) {

				// Check normalized vec between ball center and bottom-right corner
				Vector2 bottom_left_corner = {
					state->right_paddle.x,
					state->right_paddle.y + paddle_height
				};
				state->ball_dir = Vector2Normalize(Vector2Subtract(state->ball_pos, bottom_left_corner));
				state->ball_pos = Vector2Add(bottom_left_corner, Vector2Scale(state->ball_dir, ball_radius));
			
			// bottom
			} else {

				// Reflect down vector
				state->ball_dir = Vector2Reflect(state->ball_dir, DOWN_NORMAL);
			}

		// left
		} else {
			state->ball_dir = Vector2Reflect(state->ball_dir, LEFT_NORMAL);
		}
	}

	// ? Rendering
	BeginDrawing();
	ClearBackground(BLACK);

	// Draw the ball
	DrawCircle((int)state->ball_pos.x, (int)state->ball_pos.y, ball_radius, WHITE);

	// Draw the left paddle
	DrawRectangleRec(state->left_paddle, WHITE);

	// Draw the right paddle
	DrawRectangleRec(state->right_paddle, WHITE);

	// Draw GUI
	DrawText(TextFormat("%d", state->player_1_score), 32, 32, 40, WHITE);
	DrawText(TextFormat("%d", state->player_2_score), screen_width-32-40, 32, 40, WHITE);

	EndDrawing();
}

struct Dimensions get_dimensions_for_resolution(enum ScreenResolution res) {
	struct Dimensions dim;
	int ratio = 1;
	switch (res) {
		case R512X288:
			ratio = 2;
			break;
		case R768X432:
			ratio = 3;
			break;
	}
	dim.screen_width = ratio * 256;
	dim.screen_height = ratio * 144;
	dim.paddle_width = ratio * 6;
	dim.paddle_height = ratio * 24;
	dim.ball_radius = ratio * 5;
	return dim;
}

int main(int argc, char **argv) {
	if(argc != 1) {
		printf("%s takes no arguments.\n", argv[0]);
		return 1;
	}
	printf("This is project %s.\n", PROJECT_NAME);

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	//SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	struct Dimensions dimensions = get_dimensions_for_resolution(R768X432);
	InitWindow(dimensions.screen_width, dimensions.screen_height, PROJECT_NAME);
	SetTargetFPS(TARGET_FPS);

	struct GlobalState global_state = {
		.dimensions = dimensions,
		.current_screen = MAIN_MENU_SCREEN,
	};
	struct MainMenuState main_menu_state = {
		.selected_option = MENU_OPTION_START,
	};
	struct InGameState in_game_state = {
		.player_1_score = 0,
		.player_2_score = 0,
		.ball_pos = {
			(float)dimensions.screen_width / 2.0f,
			(float)dimensions.screen_height / 2.0f
		},
		.ball_dir = LEFT_NORMAL,
		.ball_speed = BALL_SPEED,
		.left_paddle = {
			PANEL_MARGIN,
			dimensions.screen_height / 2 - dimensions.paddle_height / 2,
			dimensions.paddle_width,
			dimensions.paddle_height
		},
		.right_paddle = {
			dimensions.screen_width - PANEL_MARGIN - dimensions.paddle_width,
			dimensions.screen_height / 2 - dimensions.paddle_height / 2,
			dimensions.paddle_width,
			dimensions.paddle_height
		},
	};
	do {
		switch (global_state.current_screen) {
			case MAIN_MENU_SCREEN:
				update_main_menu_screen(&global_state, &main_menu_state);
				break;
			case IN_GAME_SCREEN:
				update_in_game_screen(&global_state, &in_game_state);
			default:
				break;
		}
	} while (!WindowShouldClose());

	CloseWindow();
	return 0;
}
