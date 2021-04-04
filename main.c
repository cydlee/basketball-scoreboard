/**************************************************************************************************

Basketball Scoreboard 1.0
Copyright (c) 2021 Cyrus Lee

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.


Made with Raylib, by raysan5 <https://github.com/raysan5/raylib>

**************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "raylib.h"

#define NAME "Basketball Scoreboard"
#define VERSION "1.0"
#define COPYRIGHT "Copyright (c) 2021 Cyrus Lee"

// Input keys
#define KEY_START_STOP_CLOCK    KEY_SPACE
#define KEY_RESET_SHOT_CLOCK    KEY_LEFT_SHIFT
#define KEY_SOUND_BUZZER        KEY_G
#define KEY_CHANGING_HOME       KEY_H
#define KEY_CHANGING_VISITOR    KEY_V
#define KEY_CHANGE_MODE_SCORE   KEY_S
#define KEY_CHANGE_MODE_FOULS   KEY_F
#define KEY_CHANGE_MODE_TOL     KEY_T

#define NOT_SET 0
#define HOME    1
#define VISITOR 2

#define DARKDARKGRAY (Color){25, 25, 25, 255}

typedef struct Time { int ten_minutes, minutes, ten_seconds, seconds, tenth_seconds; } Time;
typedef struct DisplayBox { float x, y, width, height; } DisplayBox;
typedef enum Mode { SCORE = 0, FOULS, TOL } Mode;

int IsTimeEqual (Time time1, Time time2);
Time UpdateTime (Time time);
void DrawDigit (int digit, float posX, float posY, float width, Color color, int use_all);

static int version_flag;

int main (int argc, char* argv[])
{
	// Option Parsing
	//---------------------------------------------------------------------------------------------
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{"version", no_argument, &version_flag, 1},
			{0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long (argc, argv, "", long_options, &option_index);

		if (c == -1)
			break;

		switch (c)
		{
			case 0:
			case '?':
				break;
			default:
				abort ();
		}
	}

	// Version message
	//---------------------------------------------------------------------------------------------
	if (version_flag)
	{
		printf ("%s %s\n%s\n\n", NAME, VERSION, COPYRIGHT);
		fputs ("\
This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program. If not, see <https://www.gnu.org/licenses/>.\n\
\n\
\n\
Made with Raylib, by raysan5 <https://github.com/raysan5/raylib>\n\
", stdout);
		return EXIT_SUCCESS;
	}

	// Initialize
	//---------------------------------------------------------------------------------------------

	// Window
	SetConfigFlags (FLAG_WINDOW_RESIZABLE);
	InitWindow (1920, 1080, "Basketball Scoreboard");
	SetTargetFPS (30);

	// Drawing variables
	float screen_width;
	float screen_height;
	float border;
	int fontSize;

	DisplayBox main_clock_box;
	DisplayBox shot_clock_box;
	DisplayBox period_box;

	DisplayBox home_score_box;
	DisplayBox visitor_score_box;
	DisplayBox home_fouls_box;
	DisplayBox visitor_fouls_box;
	DisplayBox home_tol_box;
	DisplayBox visitor_tol_box;

	// Control variables
	int clock_stopped = 1;
	int add_tenth_second = 1;
	int team = HOME;
	Mode change_mode = SCORE;

	// Display variables
	Time main_clock = {0, 8, 0, 0, 0};
	Time shot_clock = {0, 0, 3, 5, 0};
	int score[] = {0, 0};
	int fouls[] = {0, 0};

	int tol[] = {5, 5};

	// Other variables
	Time time_zero = {0, 0, 0, 0, 0};
	Time time_35 = {0, 0, 3, 5, 0};
	Time time_8min = {0, 8, 0, 0, 0};

	// Audio
	InitAudioDevice ();
	Sound buzzer_sound = LoadSound ("buzzer.ogg");

	// Loop
	//---------------------------------------------------------------------------------------------
	while (!WindowShouldClose ())
	{
		// Inputs and updating data
		//-----------------------------------------------------------------------------------------

		// Clock controls
		//----------------------------------------------------------------
		// Start/stop clock
		if (IsKeyPressed (KEY_START_STOP_CLOCK))
		{
			if (clock_stopped)
				clock_stopped = 0;
			else
				clock_stopped = 1;
		}
		// Update clock
		if (!clock_stopped && !IsTimeEqual (shot_clock, time_zero))
		{
			if (add_tenth_second == 0)
			{
				main_clock = UpdateTime (main_clock);
				shot_clock = UpdateTime (shot_clock);
			}
			add_tenth_second++;
			if (add_tenth_second > 2)
				add_tenth_second = 0;
		}
		// Reset shot clock
		if (IsKeyPressed (KEY_RESET_SHOT_CLOCK))
			shot_clock = time_35;
		//----------------------------------------------------------------

		// Changing mode
		//----------------------------------------------------------------
		// Home or visitor
		if (IsKeyPressed (KEY_CHANGING_HOME))
			team = HOME;
		if (IsKeyPressed (KEY_CHANGING_VISITOR))
			team = VISITOR;
		// Score, fouls, TOL
		if (IsKeyPressed (KEY_CHANGE_MODE_SCORE))
			change_mode = SCORE;
		if (IsKeyPressed (KEY_CHANGE_MODE_FOULS))
			change_mode = FOULS;
		if (IsKeyPressed (KEY_CHANGE_MODE_TOL))
			change_mode = TOL;
		//----------------------------------------------------------------

		// Change score, fouls, TOL
		//----------------------------------------------------------------
		switch (change_mode)
		{
			case SCORE:
				if (IsKeyPressed (KEY_ONE) || IsKeyPressed (KEY_KP_1))
					score[team] += 1;
				if (IsKeyPressed (KEY_TWO) || IsKeyPressed (KEY_KP_2))
					score[team] += 2;
				if (IsKeyPressed (KEY_THREE) || IsKeyPressed (KEY_KP_3))
					score[team] += 3;
				if (IsKeyPressed (KEY_EQUAL) || IsKeyPressed (KEY_KP_ADD))
					score[team]++;
				if ((IsKeyPressed (KEY_MINUS) || IsKeyPressed (KEY_KP_SUBTRACT)) && score[team] > 0)
					score[team]--;
				break;
			case FOULS:
				if (IsKeyPressed (KEY_EQUAL) || IsKeyPressed (KEY_KP_ADD))
					fouls[team]++;
				if ((IsKeyPressed (KEY_MINUS) || IsKeyPressed (KEY_KP_SUBTRACT)) && fouls[team] > 0)
					fouls[team]--;
				break;
			case TOL:
				if (IsKeyPressed (KEY_EQUAL) || IsKeyPressed (KEY_KP_ADD))
					tol[team]++;
				if ((IsKeyPressed (KEY_MINUS) || IsKeyPressed (KEY_KP_SUBTRACT)) && tol[team] > 0)
					tol[team]--;
				break;
		}
		//----------------------------------------------------------------

		// Game buzzer sound
		//----------------------------------------------------------------
		if (IsKeyDown (KEY_SOUND_BUZZER))
		{
			if (!IsSoundPlaying (buzzer_sound))
				PlaySound (buzzer_sound);
		}
		else if (!clock_stopped && (IsTimeEqual (main_clock, time_zero) || IsTimeEqual (shot_clock, time_zero)))
		{
			if (!IsSoundPlaying (buzzer_sound))
				PlaySound (buzzer_sound);
		}
		else
			StopSound (buzzer_sound);
		//----------------------------------------------------------------

		//-----------------------------------------------------------------------------------------


		// Drawing
		//-----------------------------------------------------------------------------------------

		BeginDrawing ();

			// Update core display variables
			screen_width = (float) GetScreenWidth ();
			screen_height = (float) GetScreenHeight ();
			border = screen_width / 96;
			fontSize = (int) border * 6;
			// Draw background rectangle + outline
			ClearBackground (WHITE);
			DrawRectangle (border, border, screen_width - (border * 2), screen_height - (border * 2), DARKBLUE);

			// Main Clock
			//-------------------------------------------------------------------------------------
			// Update box
			main_clock_box.width = border * 29;
			main_clock_box.height = border * 11;
			main_clock_box.x = (screen_width / 2) - (main_clock_box.width / 2);
			main_clock_box.y = border;
			// Draw boxes
			DrawRectangle (main_clock_box.x - border, main_clock_box.y - border, main_clock_box.width + (border * 2), main_clock_box.height + (border * 2), WHITE);
			DrawRectangle (main_clock_box.x, main_clock_box.y, main_clock_box.width, main_clock_box.height, BLACK);
			// Draw digits
			if (main_clock.ten_minutes == 0 && main_clock.minutes == 0)
			{
				// Less than one minute
				if (main_clock.ten_seconds == 0)
					DrawDigit (11, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				else
					DrawDigit (main_clock.ten_seconds, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				DrawDigit (main_clock.seconds, main_clock_box.x + (border * 7.5f), border * 2, border * 5, RED, 1);
				DrawDigit (main_clock.tenth_seconds, main_clock_box.x + (border * 16.5f), border * 2, border * 5, RED, 1);
				DrawDigit (11, main_clock_box.x + (border * 23), border * 2, border * 5, RED, 1);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 2.5f), border, border, DARKDARKGRAY);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 7.5f), border, border, RED);
			}
			else
			{
				// More than one minute
				if (main_clock.ten_minutes == 0)
					DrawDigit (11, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				else
					DrawDigit (main_clock.ten_minutes, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				DrawDigit (main_clock.minutes, main_clock_box.x + (border * 7.5f), border * 2, border * 5, RED, 1);
				DrawDigit (main_clock.ten_seconds, main_clock_box.x + (border * 16.5f), border * 2, border * 5, RED, 1);
				DrawDigit (main_clock.seconds, main_clock_box.x + (border * 23), border * 2, border * 5, RED, 1);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 2.5f), border, border, RED);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 7.5f), border, border, RED);
			}
			//-------------------------------------------------------------------------------------
			
			// Shot clock
			//-------------------------------------------------------------------------------------
			// Update box
			shot_clock_box.width = border * 15;
			shot_clock_box.height = border * 11;
			shot_clock_box.x = (screen_width / 2) - (shot_clock_box.width / 2);
			shot_clock_box.y = main_clock_box.y + main_clock_box.height + (border * 5);
			// Draw boxes
			DrawRectangle (shot_clock_box.x - border, shot_clock_box.y - border, shot_clock_box.width + (border * 2), shot_clock_box.height + (border * 2), WHITE);
			DrawRectangle (shot_clock_box.x, shot_clock_box.y, shot_clock_box.width, shot_clock_box.height, BLACK);
			// Draw digits
			if (shot_clock.ten_seconds == 0)
			{
				// Less than ten seconds
				DrawDigit (shot_clock.seconds, shot_clock_box.x + border, shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawDigit (shot_clock.tenth_seconds, shot_clock_box.x + (border * 9), shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawRectangle (shot_clock_box.x + (border * 7), shot_clock_box.y + (border * 7.5f), border, border, GREEN);
			}
			else
			{
				// More than ten seconds
				DrawDigit (shot_clock.ten_seconds, shot_clock_box.x + border, shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawDigit (shot_clock.seconds, shot_clock_box.x + (border * 9), shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawRectangle (shot_clock_box.x + (border * 7), shot_clock_box.y + (border * 7.5f), border, border, DARKDARKGRAY);
			}
			//-------------------------------------------------------------------------------------

			// Scoreboards
			//-------------------------------------------------------------------------------------
			// Home/visitor labels
			DrawText
			(
				"HOME",
				(int) ((main_clock_box.x / 2) - ((float) MeasureText ("HOME", fontSize) / 2)),
				(int) ((((border * 2) + main_clock_box.y + main_clock_box.height) / 2) - (fontSize / 2)),
				fontSize,
				WHITE
			);
			DrawText
			(
				"VISITOR",
				(int) ((((screen_width - border) + (main_clock_box.x + main_clock_box.width + border)) / 2) - ((float) MeasureText ("VISITOR", fontSize) / 2)),
				(int) ((((border * 2) + main_clock_box.y + main_clock_box.height) / 2) - (fontSize / 2)),
				fontSize,
				WHITE
			);
			//-------------------------------------------------------------------------------------

		EndDrawing ();

	}

	// De-Initialization
	//---------------------------------------------------------------------------------------------

	// Audio
	UnloadSound (buzzer_sound);
	CloseAudioDevice ();

	// Window
	CloseWindow ();

	return EXIT_SUCCESS;
}

int IsTimeEqual (Time time1, Time time2)
{
	if (time1.ten_minutes == time2.ten_minutes && time1.minutes == time2.minutes && time1.ten_seconds == time2.ten_seconds && time1.seconds == time2.seconds && time1.tenth_seconds == time2.tenth_seconds)
		return 1;
	else
		return 0;
}

Time UpdateTime (Time time)
{
	if (time.ten_minutes == 0 && time.minutes == 0 && time.ten_seconds == 0 && time.seconds == 0 && time.tenth_seconds == 0)
		return time;
	else if (time.tenth_seconds == 0)
	{
		time.tenth_seconds = 9;
		if (time.seconds == 0)
		{
			time.seconds = 9;
			if (time.ten_seconds == 0)
			{
				time.ten_seconds = 5;
				if (time.minutes == 0)
				{
					time.minutes = 9;
					if (time.ten_minutes != 0)
						time.ten_minutes--;
				}
				else
					time.minutes--;
			}
			else
				time.ten_seconds--;
		}
		else
			time.seconds--;
	}
	else
		time.tenth_seconds--;
	return time;
}

void DrawDigit (int digit, float posX, float posY, float width, Color color, int use_all)
{
	/**********************************************************************************************

	Draws a digit for the scoreboard relative to the start position {posX, posY} and (width).
	Each rectangle in the digit has dimensions (1/5 width) x (3/5 width).

	              x
	|---------------------------|

	|-----|---------------|-----|  -
	|     |***************|     |  | 0.2x
	|     |***************|     |  |
	|-----|---------------|-----|  -
	|*****|               |*****|  |
	|*****|               |*****|  |
	|*****|               |*****|  | 0.6x
	|*****|               |*****|  |
	|*****|               |*****|  |
	|*****|               |*****|  |
	|-----|---------------|-----|  -
	|     |***************|     |
	|     |***************|     |
	|-----|---------------|-----|
	|*****|               |*****|
	|*****|               |*****|
	|*****|               |*****|  -- All of the rectangles are congruent.
	|*****|               |*****|
	|*****|               |*****|
	|*****|               |*****|
	|-----|---------------|-----|
	|     |***************|     |
	|     |***************|     |
	|-----|---------------|-----|

	**********************************************************************************************/
	// Rectangle dimensions (vertical)
	float rect_width = width / 5;
	float rect_height = rect_width * 3;
	// Rectangles
	Rectangle top = {posX + rect_width, posY, rect_height, rect_width};
	Rectangle middle = {posX + rect_width, posY + rect_width + rect_height, rect_height, rect_width};
	Rectangle bottom = {posX + rect_width, posY + (rect_width * 2) + (rect_height * 2), rect_height, rect_width};
	Rectangle top_left = {posX, posY + rect_width, rect_width, rect_height};
	Rectangle bottom_left = {posX, posY + (rect_width * 2) + rect_height, rect_width, rect_height};
	Rectangle top_right = {posX + rect_width + rect_height, posY + rect_width, rect_width, rect_height};
	Rectangle bottom_right = {posX + rect_width + rect_height, posY + (rect_width * 2) + rect_height, rect_width, rect_height};
	// Corners
	Rectangle top_left_corner = {posX, posY, rect_width, rect_width};
	Rectangle middle_left_corner = {posX, posY + rect_width + rect_height, rect_width, rect_width};
	Rectangle bottom_left_corner = {posX, posY + (rect_width * 2) + (rect_height * 2), rect_width, rect_width};
	Rectangle top_right_corner = {posX + rect_width + rect_height, posY, rect_width, rect_width};
	Rectangle middle_right_corner = {posX + rect_width + rect_height, posY + rect_width + rect_height, rect_width, rect_width};
	Rectangle bottom_right_corner = {posX + rect_width + rect_height, posY + (rect_width * 2) + (rect_height * 2), rect_width, rect_width};
	// Rectangle colors set to gray (off)
	Color top_color = DARKDARKGRAY;
	Color middle_color = DARKDARKGRAY;
	Color bottom_color = DARKDARKGRAY;
	Color top_left_color = DARKDARKGRAY;
	Color bottom_left_color = DARKDARKGRAY;
	Color top_right_color = DARKDARKGRAY;
	Color bottom_right_color = DARKDARKGRAY;
	// Corner colors set to gray (off)
	Color top_left_corner_color = DARKDARKGRAY;
	Color middle_left_corner_color = DARKDARKGRAY;
	Color bottom_left_corner_color = DARKDARKGRAY;
	Color top_right_corner_color = DARKDARKGRAY;
	Color middle_right_corner_color = DARKDARKGRAY;
	Color bottom_right_corner_color = DARKDARKGRAY;
	// Set rectangles on for each digit
	switch (digit)
	{
		case 0:
			if (use_all)
			{
				// Rectangles
				top_color = color;
				bottom_color = color;
				top_left_color = color;
				bottom_left_color = color;
				top_right_color = color;
				bottom_right_color = color;
				// Corners
				top_left_corner_color = color;
				middle_left_corner_color = color;
				bottom_left_corner_color = color;
				top_right_corner_color = color;
				middle_right_corner_color = color;
				bottom_right_corner_color = color;
			}
			break;
		case 1:
			// Rectangles
			top_right_color = color;
			bottom_right_color = color;
			// Corners
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 2:
			// Rectangles
			top_color = color;
			middle_color = color;
			bottom_color = color;
			bottom_left_color = color;
			top_right_color = color;
			// Corners
			top_left_corner_color = color;
			middle_left_corner_color = color;
			bottom_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 3:
			// Rectangles
			top_color = color;
			middle_color = color;
			bottom_color = color;
			top_right_color = color;
			bottom_right_color = color;
			// Corners
			top_left_corner_color = color;
			middle_left_corner_color = color;
			bottom_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 4:
			// Rectangles
			middle_color = color;
			top_left_color = color;
			top_right_color = color;
			bottom_right_color = color;
			// Corners
			top_left_corner_color = color;
			middle_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 5:
			// Rectangles
			top_color = color;
			middle_color = color;
			bottom_color = color;
			top_left_color = color;
			bottom_right_color = color;
			// Corners
			top_left_corner_color = color;
			middle_left_corner_color = color;
			bottom_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 6:
			// Rectangles
			top_color = color;
			middle_color = color;
			bottom_color = color;
			top_left_color = color;
			bottom_left_color = color;
			bottom_right_color = color;
			// Corners
			top_left_corner_color = color;
			middle_left_corner_color = color;
			bottom_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 7:
			// Rectangles
			top_color = color;
			top_right_color = color;
			bottom_right_color = color;
			// Corners
			top_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 8:
			// Rectangles
			top_color = color;
			middle_color = color;
			bottom_color = color;
			top_left_color = color;
			bottom_left_color = color;
			top_right_color = color;
			bottom_right_color = color;
			// Corners
			top_left_corner_color = color;
			middle_left_corner_color = color;
			bottom_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
		case 9:
			// Rectangles
			top_color = color;
			middle_color = color;
			bottom_color = color;
			top_left_color = color;
			top_right_color = color;
			bottom_right_color = color;
			// Corners
			top_left_corner_color = color;
			middle_left_corner_color = color;
			bottom_left_corner_color = color;
			top_right_corner_color = color;
			middle_right_corner_color = color;
			bottom_right_corner_color = color;
			break;
	}
	// Draw Rectangles
	if (use_all)
	{
		DrawRectangleRec (top, top_color);
		DrawRectangleRec (middle, middle_color);
		DrawRectangleRec (bottom, bottom_color);
		DrawRectangleRec (top_left, top_left_color);
		DrawRectangleRec (bottom_left, bottom_left_color);
	}
	DrawRectangleRec (top_right, top_right_color);
	DrawRectangleRec (bottom_right, bottom_right_color);
	// Draw Corners
	if (use_all)
	{
		DrawRectangleRec (top_left_corner, top_left_corner_color);
		DrawRectangleRec (middle_left_corner, middle_left_corner_color);
		DrawRectangleRec (bottom_left_corner, bottom_left_corner_color);
	}
	DrawRectangleRec (top_right_corner, top_right_corner_color);
	DrawRectangleRec (middle_right_corner, middle_right_corner_color);
	DrawRectangleRec (bottom_right_corner, bottom_right_corner_color);
}

