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

#define DARKDARKGRAY (Color){25, 25, 25, 255}

typedef struct Time { int ten_minutes, minutes, ten_seconds, seconds, tenth_seconds; } Time;
typedef enum Mode { NORMAL = 0, } Mode;
typedef enum Possession { NOT_SET = 0, HOME, VISITOR } Possession;

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
	SetTargetFPS (60);

	// Variables
	float border;
	float spacer;
	int clock_stopped = 0;
	int add_tenth_second = 0;
	Time time_zero = {0, 0, 0, 0, 0};

	Time main_clock = {3, 1, 0, 0, 0};
	Time shot_clock = {0, 0, 0, 0, 0};
	int home_score = 0;
	int visitor_score = 0;
	int home_fouls = 0;
	int visitor_fouls = 0;
	int home_tol = 5;
	int visitor_tol = 5;
	int home_bonus = 0;
	int visitor_bonus = 0;
	Possession possession_arrow = NOT_SET;

	// Audio
	InitAudioDevice ();
	Sound buzzer_sound = LoadSound ("buzzer.ogg");

	// Loop
	//---------------------------------------------------------------------------------------------
	while (!WindowShouldClose ())
	{
		// Inputs and updating data
		//-----------------------------------------------------------------------------------------

		// Start/stop clock
		if (IsKeyPressed (KEY_SPACE))
		{
			if (clock_stopped)
				clock_stopped = 0;
			else
				clock_stopped = 1;
		}

		// Game buzzer sound
		if (IsKeyDown (KEY_H))
		{
			if (!IsSoundPlaying (buzzer_sound))
				PlaySound (buzzer_sound);
		}
		else if (!clock_stopped && IsTimeEqual (main_clock, time_zero))
		{
			if (!IsSoundPlaying (buzzer_sound))
				PlaySound (buzzer_sound);
		}
		else
			StopSound (buzzer_sound);

		// Update clock
		if (!clock_stopped)
		{
			if (add_tenth_second == 0)
				main_clock = UpdateTime (main_clock);
			add_tenth_second++;
			if (add_tenth_second > 5)
				add_tenth_second = 0;
		}

		// Drawing
		//-----------------------------------------------------------------------------------------
		BeginDrawing ();
			float screen_width = (float) GetScreenWidth ();
			float screen_height = (float) GetScreenHeight ();
			border = screen_width / 96;
			spacer = border;
			// Draw background rectangle + outline
			ClearBackground (WHITE);
			DrawRectangle (border, border, screen_width - (border * 2), screen_height - (border * 2), DARKBLUE);
			// Draw main clock rectangle + outline
			float main_clock_width = spacer * 29;
			float main_clock_height = spacer * 11;
			float main_clock_x = (screen_width / 2) - (main_clock_width / 2);
			float main_clock_y = border;
			DrawRectangle (main_clock_x - border, 0, main_clock_width + (border * 2), main_clock_height + (border * 2), WHITE);
			DrawRectangle (main_clock_x, main_clock_y, main_clock_width, main_clock_height, BLACK);
			// Draw main clock digits
			if (main_clock.ten_minutes == 0 && main_clock.minutes == 0)
			{
				// Less than one minute
				if (main_clock.ten_seconds == 0)
					DrawDigit (11, main_clock_x + spacer, spacer * 2, spacer * 5, RED, 1);
				else
					DrawDigit (main_clock.ten_seconds, main_clock_x + spacer, spacer * 2, spacer * 5, RED, 1);
				DrawDigit (main_clock.seconds, main_clock_x + (spacer * 7.5f), spacer * 2, spacer * 5, RED, 1);
				DrawDigit (main_clock.tenth_seconds, main_clock_x + (spacer * 16.5f), spacer * 2, spacer * 5, RED, 1);
				DrawDigit (11, main_clock_x + (spacer * 23), spacer * 2, spacer * 5, RED, 1);
				DrawRectangle (main_clock_x + (spacer * 14), main_clock_y + (spacer * 2.5f), spacer, spacer, DARKDARKGRAY);
				DrawRectangle (main_clock_x + (spacer * 14), main_clock_y + (spacer * 7.5f), spacer, spacer, RED);
			}
			else
			{
				// More than one minute
				if (main_clock.ten_minutes == 0)
					DrawDigit (11, main_clock_x + spacer, spacer * 2, spacer * 5, RED, 1);
				else
					DrawDigit (main_clock.ten_minutes, main_clock_x + spacer, spacer * 2, spacer * 5, RED, 1);
				DrawDigit (main_clock.minutes, main_clock_x + (spacer * 7.5f), spacer * 2, spacer * 5, RED,1 );
				DrawDigit (main_clock.ten_seconds, main_clock_x + (spacer * 16.5f), spacer * 2, spacer * 5, RED,1 );
				DrawDigit (main_clock.seconds, main_clock_x + (spacer * 23), spacer * 2, spacer * 5, RED, 1);
				DrawRectangle (main_clock_x + (spacer * 14), main_clock_y + (spacer * 2.5f), spacer, spacer, RED);
				DrawRectangle (main_clock_x + (spacer * 14), main_clock_y + (spacer * 7.5f), spacer, spacer, RED);
			}
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
				top_color = RED;
				bottom_color = RED;
				top_left_color = RED;
				bottom_left_color = RED;
				top_right_color = RED;
				bottom_right_color = RED;
				// Corners
				top_left_corner_color = RED;
				middle_left_corner_color = RED;
				bottom_left_corner_color = RED;
				top_right_corner_color = RED;
				middle_right_corner_color = RED;
				bottom_right_corner_color = RED;
			}
			break;
		case 1:
			// Rectangles
			top_right_color = RED;
			bottom_right_color = RED;
			// Corners
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 2:
			// Rectangles
			top_color = RED;
			middle_color = RED;
			bottom_color = RED;
			bottom_left_color = RED;
			top_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			middle_left_corner_color = RED;
			bottom_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 3:
			// Rectangles
			top_color = RED;
			middle_color = RED;
			bottom_color = RED;
			top_right_color = RED;
			bottom_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			middle_left_corner_color = RED;
			bottom_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 4:
			// Rectangles
			middle_color = RED;
			top_left_color = RED;
			top_right_color = RED;
			bottom_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			middle_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 5:
			// Rectangles
			top_color = RED;
			middle_color = RED;
			bottom_color = RED;
			top_left_color = RED;
			bottom_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			middle_left_corner_color = RED;
			bottom_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 6:
			// Rectangles
			top_color = RED;
			middle_color = RED;
			bottom_color = RED;
			top_left_color = RED;
			bottom_left_color = RED;
			bottom_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			middle_left_corner_color = RED;
			bottom_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 7:
			// Rectangles
			top_color = RED;
			top_right_color = RED;
			bottom_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 8:
			// Rectangles
			top_color = RED;
			middle_color = RED;
			bottom_color = RED;
			top_left_color = RED;
			bottom_left_color = RED;
			top_right_color = RED;
			bottom_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			middle_left_corner_color = RED;
			bottom_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
			break;
		case 9:
			// Rectangles
			top_color = RED;
			middle_color = RED;
			bottom_color = RED;
			top_left_color = RED;
			top_right_color = RED;
			bottom_right_color = RED;
			// Corners
			top_left_corner_color = RED;
			middle_left_corner_color = RED;
			bottom_left_corner_color = RED;
			top_right_corner_color = RED;
			middle_right_corner_color = RED;
			bottom_right_corner_color = RED;
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

