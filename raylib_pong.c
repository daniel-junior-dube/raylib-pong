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

struct ObjectDimensions {
	int screen_width;
	int screen_height;
	int paddle_width;
	int paddle_height;
	int ball_radius;
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
	struct ObjectDimensions object_dimensions;
	enum Screen current_screen;
};

struct MainMenuState {
	enum MenuOption selected_option;
};

struct InGameState {
	int player_1_score;
	int player_2_score;
	Vector2 ball_pos;
	Vector2 ball_vel;
	Rectangle left_paddle;
	Rectangle right_paddle;
};

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
	int screen_width = global_state->object_dimensions.screen_width;
	int screen_height = global_state->object_dimensions.screen_height;
	int paddle_height = global_state->object_dimensions.paddle_height;
	int paddle_width = global_state->object_dimensions.paddle_width;
	int ball_radius = global_state->object_dimensions.ball_radius;
	// ? Processing inputs
	// ...
	if (IsKeyDown(KEY_UP)) {
		state->left_paddle.y -= 3.0f;
		if (state->left_paddle.y < 0.0f) {
			state->left_paddle.y = 0.0f;
		}
	}
	if (IsKeyDown(KEY_DOWN)) {
		state->left_paddle.y += 3.0f;
		float max_paddle_y = screen_height - paddle_height;
		if (state->left_paddle.y > max_paddle_y) {
			state->left_paddle.y = max_paddle_y;
		}
	}

	// ? Updating
	state->ball_pos = Vector2Add(state->ball_pos, state->ball_vel);
	if ((int)state->ball_pos.y >= screen_height - ball_radius) {
		Vector2 normal = { 0.0f, 1.0f };
		state->ball_vel = Vector2Reflect(state->ball_vel, normal);
	}
	if ((int)state->ball_pos.y <= ball_radius) {
		Vector2 normal = { 0.0f, -1.0f };
		state->ball_vel = Vector2Reflect(state->ball_vel, normal);
	}
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
	if (state->ball_vel.x < 0.0f && CheckCollisionCircleRec(state->ball_pos, ball_radius, state->left_paddle)) {
		Vector2 normal = { 1.0f, 0.0f };
		state->ball_vel = Vector2Reflect(state->ball_vel, normal);
	}
	if (state->ball_vel.x > 0.0f && CheckCollisionCircleRec(state->ball_pos, ball_radius, state->right_paddle)) {
		Vector2 normal = { -1.0f, 0.0f };
		state->ball_vel = Vector2Reflect(state->ball_vel, normal);
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

enum ScreenResolution {
	R256X144,
	R512X288,
};

struct ObjectDimensions get_dimensions_for_resolution(enum ScreenResolution res) {
	struct ObjectDimensions dim;
	dim.screen_width = 256;
	dim.screen_height = 144;
	switch (res) {
		case R512X288:
			dim.screen_width *= 2;
			dim.screen_height *= 2;
			break;
	}
	int ratio = dim.screen_width / 256;
	dim.paddle_width = ratio * 4;
	dim.paddle_height = ratio * 24;
	dim.ball_radius = ratio * 3;
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

	struct ObjectDimensions dimensions = get_dimensions_for_resolution(R512X288);
	InitWindow(dimensions.screen_width, dimensions.screen_height, PROJECT_NAME);
	SetTargetFPS(TARGET_FPS);

	struct GlobalState global_state = {
		.object_dimensions = dimensions,
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
		.ball_vel = { 5.0f, 0.0f },
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
