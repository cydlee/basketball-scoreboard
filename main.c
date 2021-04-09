/**************************************************************************************************

Basketball Scoreboard version 1
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

===================================================================================================

TABLE OF CONTENTS:
# Option parsing
# Version message
# Initialize
# Loop
	## Update board logic
		### Clock running
		### Clock stopped
		### Changing mode
		### Change score, fouls, TOL
		### Game buzzer sound
		### Edit mode
	## Drawing
		### Main clock
		### Shot clock
		### Period
		### Score displays
		### Foul displays
		### TOL displays
# De-initialization

**************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "raylib.h"

#define NAME "Basketball Scoreboard"
#define VERSION "version 1"
#define COPYRIGHT "Copyright (c) 2021 Cyrus Lee"

// Input keys
#define KEY_TOGGLE_FULLSCREEN   KEY_F11
#define KEY_START_STOP_CLOCK    KEY_SPACE
#define KEY_RESET_SHOT_CLOCK    KEY_LEFT_SHIFT
#define KEY_SOUND_BUZZER        KEY_G
#define KEY_CHANGING_HOME       KEY_H
#define KEY_CHANGING_VISITOR    KEY_V
#define KEY_CHANGE_MODE_PERIOD  KEY_P
#define KEY_CHANGE_MODE_SCORE   KEY_S
#define KEY_CHANGE_MODE_FOULS   KEY_F
#define KEY_CHANGE_MODE_TOL     KEY_T
// Increment, decrement, score +1/2/3 keybinds are under [Change score, fouls, TOL]
// Edit mode keybinds are under [Edit mode]

#define HOME    0
#define VISITOR 1

#define DARKDARKGRAY (Color){25, 25, 25, 255}
#define DARKRED (Color){130, 33, 55, 255}

typedef struct Time { int ten_minutes, minutes, ten_seconds, seconds, tenth_seconds; } Time;
typedef struct DisplayBox { float x, y, width, height; } DisplayBox;
typedef enum ChangeType { SCORE = 0, FOULS, TOL, PERIOD } ChangeType;
typedef enum TimerMode { NORMAL = 0, TENTH_SECONDS } TimerMode;
typedef enum Mode { CLOCK_STOPPED = 0, CLOCK_RUNNING, EDIT_MODE } Mode;

int TimeToInt (Time time); // Returns time in tenths of seconds (int)
Time UpdateTime (Time time); // Increments time by one tenth second
void DrawDigit (int digit, float posX, float posY, float width, Color color, int use_all); // Draw a digit on the scoreboard

static int version_flag;

int main (int argc, char* argv[])
{
	// # Option parsing
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
	//---------------------------------------------------------------------------------------------


	// # Version message
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
Made with Raylib, by raysan5 <https://github.com/raysan5/raylib>\n\
", stdout);
		return EXIT_SUCCESS;
	}
	//---------------------------------------------------------------------------------------------


	// # Initialize
	//---------------------------------------------------------------------------------------------

	// Window
	SetConfigFlags (FLAG_WINDOW_RESIZABLE);
	InitWindow (1920, 1080, "Basketball Scoreboard");
	SetTargetFPS (30);

	// Window icon
	Image window_icon = LoadImage ("icon.png");
	SetWindowIcon (window_icon);

	// Drawing variables
	float screen_width, screen_height, border;
	int fontSize, period_label_y, hvlabel_y, fouls_label_y;

	DisplayBox main_clock_box, shot_clock_box, period_box;

	DisplayBox home_score_box;
	DisplayBox visitor_score_box;
	DisplayBox home_fouls_box;
	DisplayBox visitor_fouls_box;
	DisplayBox home_tol_box;
	DisplayBox visitor_tol_box;

	// Control variables
	int add_tenth_second = 1;
	int team = HOME;
	ChangeType change_type = SCORE;
	TimerMode main_clock_mode = NORMAL;
	TimerMode shot_clock_mode = NORMAL;
	Mode scoreboard_mode = CLOCK_STOPPED;

	// Game data
	Time main_clock = {0, 8, 0, 0, 0};
	Time shot_clock = {0, 0, 3, 5, 0};
	int score[] = {0, 0};
	int fouls[] = {0, 0};
	int tol[] = {5, 5};
	int period = 0;

	// Display times
	Time main_clock_display = main_clock;
	Time shot_clock_display = shot_clock;

	// Edit mode buffers + pointer
	Time main_clock_buffer = main_clock;
	Time shot_clock_buffer = shot_clock;
	int score_buffer[] = {0, 0};
	int selected_digit = 1;
	int replace_digit = -1;

	// Other variables
	Time time_35 = {0, 0, 3, 5, 0};

	// Audio
	InitAudioDevice ();
	Sound buzzer_sound = LoadSound ("buzzer.ogg");

	//---------------------------------------------------------------------------------------------


	// # Loop
	//---------------------------------------------------------------------------------------------
	while (!WindowShouldClose ())
	{
		// ## Update board logic
		//-----------------------------------------------------------------------------------------
		// Toggle fullscreen
		if (IsKeyPressed (KEY_TOGGLE_FULLSCREEN))
			ToggleFullscreen ();

		switch (scoreboard_mode)
		{
			case CLOCK_RUNNING:
				// ### Clock running
				//----------------------------------------------------------------
				// Update clock
				if (TimeToInt (shot_clock) != 0 && TimeToInt (main_clock) != 0)
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
				//----------------------------------------------------------------
			case CLOCK_STOPPED:
				// ### Clock stopped
				//----------------------------------------------------------------
				// Start/stop clock
				if (IsKeyPressed (KEY_START_STOP_CLOCK))
				{
					if (scoreboard_mode == CLOCK_STOPPED)
						scoreboard_mode = CLOCK_RUNNING;
					else
						scoreboard_mode = CLOCK_STOPPED;
				}
				
				// Reset shot clock
				if (IsKeyPressed (KEY_RESET_SHOT_CLOCK))
					shot_clock = time_35;
				// Check if either clock should be set to tenth_seconds mode
				if (TimeToInt (main_clock) < 600)
					main_clock_mode = TENTH_SECONDS;
				else
					main_clock_mode = NORMAL;
				if (TimeToInt (shot_clock) < 100)
					shot_clock_mode = TENTH_SECONDS;
				else
					shot_clock_mode = NORMAL;
				//----------------------------------------------------------------

				// ### Changing mode
				//----------------------------------------------------------------
				// Home or visitor
				if (IsKeyPressed (KEY_CHANGING_HOME))
					team = HOME;
				if (IsKeyPressed (KEY_CHANGING_VISITOR))
					team = VISITOR;
				// Score, fouls, TOL, period
				if (IsKeyPressed (KEY_CHANGE_MODE_SCORE))
					change_type = SCORE;
				if (IsKeyPressed (KEY_CHANGE_MODE_FOULS))
					change_type = FOULS;
				if (IsKeyPressed (KEY_CHANGE_MODE_TOL))
					change_type = TOL;
				if (IsKeyPressed (KEY_CHANGE_MODE_PERIOD))
					change_type = PERIOD;
				//----------------------------------------------------------------

				// ### Change score, fouls, TOL
				//----------------------------------------------------------------
				switch (change_type)
				{
					case SCORE:
						if ((IsKeyPressed (KEY_ONE) || IsKeyPressed (KEY_KP_1)) && score[team] + 1 < 200)
							score[team] += 1;
						if ((IsKeyPressed (KEY_TWO) || IsKeyPressed (KEY_KP_2)) && score[team] + 2 < 200)
							score[team] += 2;
						if ((IsKeyPressed (KEY_THREE) || IsKeyPressed (KEY_KP_3)) && score[team] + 3 < 200)
							score[team] += 3;
						if ((IsKeyPressed (KEY_EQUAL) || IsKeyPressed (KEY_KP_ADD)) && score[team] + 1 < 200)
							score[team]++;
						if ((IsKeyPressed (KEY_MINUS) || IsKeyPressed (KEY_KP_SUBTRACT)) && score[team] > 0)
							score[team]--;
						break;
					case FOULS:
						if ((IsKeyPressed (KEY_EQUAL) || IsKeyPressed (KEY_KP_ADD)) && fouls[team] < 19)
							fouls[team]++;
						if ((IsKeyPressed (KEY_MINUS) || IsKeyPressed (KEY_KP_SUBTRACT)) && fouls[team] > 0)
							fouls[team]--;
						break;
					case TOL:
						if ((IsKeyPressed (KEY_EQUAL) || IsKeyPressed (KEY_KP_ADD)) && tol[team] < 9)
							tol[team]++;
						if ((IsKeyPressed (KEY_MINUS) || IsKeyPressed (KEY_KP_SUBTRACT)) && tol[team] > 0)
							tol[team]--;
						break;
					case PERIOD:
						if ((IsKeyPressed (KEY_EQUAL) || IsKeyPressed (KEY_KP_ADD)) && period < 9)
							period++;
						if ((IsKeyPressed (KEY_MINUS) || IsKeyPressed (KEY_KP_SUBTRACT)) && period > -1)
							period--;
						break;
				}
				//----------------------------------------------------------------

				// ### Game buzzer sound
				//----------------------------------------------------------------
				if (IsKeyDown (KEY_SOUND_BUZZER))
				{
					if (!IsSoundPlaying (buzzer_sound))
						PlaySound (buzzer_sound);
				}
				else if (scoreboard_mode == CLOCK_RUNNING && (TimeToInt (main_clock) == 0 || TimeToInt (shot_clock) == 0))
				{
					if (!IsSoundPlaying (buzzer_sound))
						PlaySound (buzzer_sound);
				}
				else
					StopSound (buzzer_sound);
				//----------------------------------------------------------------

				// Update edit mode buffers and enter edit mode
				if (scoreboard_mode == CLOCK_STOPPED && (IsKeyPressed (KEY_SLASH) || IsKeyPressed (KEY_BACKSLASH)))
				{
					main_clock_buffer = main_clock;
					shot_clock_buffer = shot_clock;
					score_buffer[HOME] = score[HOME];
					score_buffer[VISITOR] = score[VISITOR];
					selected_digit = 1; // 1-4 = digits 1-4 of the main clock, 5-6 = digits 1-2 of the shot clock
					replace_digit = -1;
					scoreboard_mode = EDIT_MODE;
				}

				// Set clock displays to actual time
				main_clock_display = main_clock;
				shot_clock_display = shot_clock;
				break;
			case EDIT_MODE:
				// ### Edit mode
				//---------------------------------------------------------------------------------
				// Discard changes and exit edit mode
				if (IsKeyPressed (KEY_BACKSPACE))
					scoreboard_mode = CLOCK_STOPPED;
				// Save changes and exit edit mode
				if (IsKeyPressed (KEY_ENTER))
				{
					main_clock = main_clock_buffer;
					shot_clock = shot_clock_buffer;
					score[HOME] = score_buffer[HOME];
					score[VISITOR] = score_buffer[VISITOR];
					scoreboard_mode = CLOCK_STOPPED;
				}
				// Switch selected digit
				if (IsKeyPressed (KEY_LEFT) && selected_digit > 1)
					selected_digit--;
				if (IsKeyPressed (KEY_RIGHT) && selected_digit < 6)
					selected_digit++;
				// Conveniences to jump to shot or main clock quickly (first digit)
				if (IsKeyPressed (KEY_UP))
					selected_digit = 1;
				if (IsKeyPressed (KEY_DOWN))
					selected_digit = 5;
				// Flip mode between normal and tenth seconds
				if (IsKeyPressed (KEY_LEFT_SHIFT))
				{
					switch (selected_digit)
					{
						case 1:
						case 2:
						case 3:
						case 4:
							if (main_clock_mode == NORMAL)
								main_clock_mode = TENTH_SECONDS;
							else
								main_clock_mode = NORMAL;
							break;
						case 5:
						case 6:
							if (shot_clock_mode == NORMAL)
								shot_clock_mode = TENTH_SECONDS;
							else
								shot_clock_mode = NORMAL;
							break;
					}
				}
				// Change numbers - I cannot find a way to avoid this ugly if-else blob, please make a
				//                  pull request or issue or something if you find something
				if (IsKeyPressed (KEY_ONE) || IsKeyPressed (KEY_KP_1))
					replace_digit = 1;
				else if (IsKeyPressed (KEY_TWO) || IsKeyPressed (KEY_KP_2))
					replace_digit = 2;
				else if (IsKeyPressed (KEY_THREE) || IsKeyPressed (KEY_KP_3))
					replace_digit = 3;
				else if (IsKeyPressed (KEY_FOUR) || IsKeyPressed (KEY_KP_4))
					replace_digit = 4;
				else if (IsKeyPressed (KEY_FIVE) || IsKeyPressed (KEY_KP_5))
					replace_digit = 5;
				else if (IsKeyPressed (KEY_SIX) || IsKeyPressed (KEY_KP_6))
					replace_digit = 6;
				else if (IsKeyPressed (KEY_SEVEN) || IsKeyPressed (KEY_KP_7))
					replace_digit = 7;
				else if (IsKeyPressed (KEY_EIGHT) || IsKeyPressed (KEY_KP_8))
					replace_digit = 8;
				else if (IsKeyPressed (KEY_NINE) || IsKeyPressed (KEY_KP_9))
					replace_digit = 9;
				else if (IsKeyPressed (KEY_ZERO) || IsKeyPressed (KEY_KP_0))
					replace_digit = 0;
				else
					replace_digit = -1;
				// Move the selected number into the appropriate place in the edit mode buffers
				// This if-switch-if-else-switch thing is really ugly too (╯°□°)╯︵ ┻━┻
				// Really, this code is disgusting and it hurts to write it but I can't think of a better solution
				if (replace_digit != -1)
				{
					switch (selected_digit)
					{
						case 1:
						case 2:
						case 3:
						case 4:
							if (main_clock_mode == NORMAL)
							{
								switch (selected_digit)
								{
									case 1:
										main_clock_buffer.ten_minutes = replace_digit;
										break;
									case 2:
										main_clock_buffer.minutes = replace_digit;
										break;
									case 3:
										if (replace_digit < 6)
											main_clock_buffer.ten_seconds = replace_digit;
										break;
									case 4:
										main_clock_buffer.seconds = replace_digit;
										break;
								}
							}
							else
							{
								switch (selected_digit)
								{
									case 1:
										if (replace_digit < 6)
											main_clock_buffer.ten_seconds = replace_digit;
										break;
									case 2:
										main_clock_buffer.seconds = replace_digit;
										break;
									case 3:
										main_clock_buffer.tenth_seconds = replace_digit;
										break;
									// No case 4: because when it is in tenth seconds mode, the last digit represents nothing
								}
							}
							break;
						case 5:
						case 6:
							if (shot_clock_mode == NORMAL)
							{
								switch (selected_digit)
								{
									case 5:
										shot_clock_buffer.ten_seconds = replace_digit;
										break;
									case 6:
										shot_clock_buffer.seconds = replace_digit;
										break;
								}
							}
							else
							{
								switch (selected_digit)
								{
									case 5:
										shot_clock_buffer.seconds = replace_digit;
										break;
									case 6:
										shot_clock_buffer.tenth_seconds = replace_digit;
										break;
								}
							}
							break;
					}
				}
				// Set display time to edit mode buffer
				main_clock_display = main_clock_buffer;
				shot_clock_display = shot_clock_buffer;
				//---------------------------------------------------------------------------------
				break;
		}
		//-----------------------------------------------------------------------------------------


		// ## Drawing
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

			// ### Main clock
			//-------------------------------------------------------------------------------------
			// Update box
			main_clock_box.width = border * 29;
			main_clock_box.height = border * 11;
			main_clock_box.x = (screen_width / 2) - (main_clock_box.width / 2);
			main_clock_box.y = border;
			// Draw boxes
			if (scoreboard_mode == CLOCK_RUNNING)
				DrawRectangle (main_clock_box.x - border, main_clock_box.y - border, main_clock_box.width + (border * 2), main_clock_box.height + (border * 2), WHITE);
			else
				DrawRectangle (main_clock_box.x - border, main_clock_box.y - border, main_clock_box.width + (border * 2), main_clock_box.height + (border * 2), RED);
			DrawRectangle (main_clock_box.x, main_clock_box.y, main_clock_box.width, main_clock_box.height, BLACK);
			// Edit mode
			if (scoreboard_mode == EDIT_MODE)
			{
				switch (selected_digit)
				{
					case 1:
						DrawRectangle (main_clock_box.x, border, border * 7, border * 11, DARKRED);
						break;
					case 2:
						DrawRectangle (main_clock_box.x + (border * 6.5f), border, border * 7, border * 11, DARKRED);
						break;
					case 3:
						DrawRectangle (main_clock_box.x + (border * 15.5f), border, border * 7, border * 11, DARKRED);
						break;
					case 4:
						DrawRectangle (main_clock_box.x + (border * 22), border, border * 7, border * 11, DARKRED);
						break;
				}
			}
			// Draw digits
			if (main_clock_mode == TENTH_SECONDS)
			{
				// Less than one minute
				if (main_clock_display.ten_seconds == 0)
					DrawDigit (-1, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				else
					DrawDigit (main_clock_display.ten_seconds, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				DrawDigit (main_clock_display.seconds, main_clock_box.x + (border * 7.5f), border * 2, border * 5, RED, 1);
				DrawDigit (main_clock_display.tenth_seconds, main_clock_box.x + (border * 16.5f), border * 2, border * 5, RED, 1);
				DrawDigit (-1, main_clock_box.x + (border * 23), border * 2, border * 5, RED, 1);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 2.5f), border, border, DARKDARKGRAY);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 7.5f), border, border, RED);
			}
			else
			{
				// More than one minute
				if (main_clock_display.ten_minutes == 0)
					DrawDigit (-1, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				else
					DrawDigit (main_clock_display.ten_minutes, main_clock_box.x + border, border * 2, border * 5, RED, 1);
				DrawDigit (main_clock_display.minutes, main_clock_box.x + (border * 7.5f), border * 2, border * 5, RED, 1);
				DrawDigit (main_clock_display.ten_seconds, main_clock_box.x + (border * 16.5f), border * 2, border * 5, RED, 1);
				DrawDigit (main_clock_display.seconds, main_clock_box.x + (border * 23), border * 2, border * 5, RED, 1);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 2.5f), border, border, RED);
				DrawRectangle (main_clock_box.x + (border * 14), main_clock_box.y + (border * 7.5f), border, border, RED);
			}
			//-------------------------------------------------------------------------------------
			
			// ### Shot clock
			//-------------------------------------------------------------------------------------
			// Update box
			shot_clock_box.width = border * 14;
			shot_clock_box.height = border * 11;
			shot_clock_box.x = (screen_width / 2) - (shot_clock_box.width / 2);
			shot_clock_box.y = screen_height - shot_clock_box.height - (border * 5);
			// Draw boxes
			DrawRectangle (shot_clock_box.x - border, shot_clock_box.y - border, shot_clock_box.width + (border * 2), shot_clock_box.height + (border * 2), WHITE);
			DrawRectangle (shot_clock_box.x, shot_clock_box.y, shot_clock_box.width, shot_clock_box.height, BLACK);
			// Edit mode
			if (scoreboard_mode == EDIT_MODE)
			{
				switch (selected_digit)
				{
					case 5:
						DrawRectangle (shot_clock_box.x, shot_clock_box.y, border * 7, border * 11, DARKGREEN);
						break;
					case 6:
						DrawRectangle (shot_clock_box.x + (border * 7), shot_clock_box.y, border * 7, border * 11, DARKGREEN);
						break;
				}
			}
			// Draw digits
			if (shot_clock_mode == TENTH_SECONDS)
			{
				// Less than ten seconds
				DrawDigit (shot_clock_display.seconds, shot_clock_box.x + border, shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawDigit (shot_clock_display.tenth_seconds, shot_clock_box.x + (border * 8), shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawRectangle (shot_clock_box.x + (border * 6.5f), shot_clock_box.y + (border * 7.5f), border, border, GREEN);
			}
			else
			{
				// More than ten seconds
				DrawDigit (shot_clock_display.ten_seconds, shot_clock_box.x + border, shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawDigit (shot_clock_display.seconds, shot_clock_box.x + (border * 8), shot_clock_box.y + border, border * 5, GREEN, 1);
				DrawRectangle (shot_clock_box.x + (border * 6.5f), shot_clock_box.y + (border * 7.5f), border, border, DARKDARKGRAY);
			}
			//-------------------------------------------------------------------------------------

			// ### Period
			//-------------------------------------------------------------------------------------
			// Update box
			period_box.width = border * 7;
			period_box.height = border * 11;
			period_label_y = (int) ((((shot_clock_box.y - border) + (main_clock_box.y + main_clock_box.height + border)) / 2) - ((period_box.height + border + (fontSize / 3)) / 2));
			period_box.x = (screen_width / 2) - (period_box.width / 2);
			period_box.y = (float) period_label_y + (fontSize / 3) + border;
			// Draw label
			DrawText
			(
				"PERIOD",
				(int) ((period_box.x + (period_box.width / 2)) - ((float) MeasureText ("PERIOD", fontSize / 3) / 2)),
				period_label_y,
				fontSize / 3,
				WHITE
			);
			// Draw boxes
			DrawRectangle (period_box.x - border, period_box.y - border, period_box.width + (border * 2), period_box.height + (border * 2), WHITE);
			DrawRectangle (period_box.x, period_box.y, period_box.width, period_box.height, BLACK);
			// Draw digit
			DrawDigit (period, period_box.x + border, period_box.y + border, border * 5, ORANGE, 1);
			//-------------------------------------------------------------------------------------

			// ### Score displays
			//-------------------------------------------------------------------------------------
			// Label y value
			hvlabel_y = (int) ((((border * 2) + main_clock_box.y + main_clock_box.height) / 2) - (fontSize / 2));

			// Home label
			DrawText
			(
				"HOME",
				(int) ((main_clock_box.x / 2) - ((float) MeasureText ("HOME", fontSize) / 2)),
				hvlabel_y,
				fontSize,
				WHITE
			);
			// Update home score box
			home_score_box.width = border * 16;
			home_score_box.height = border * 11;
			home_score_box.x = (main_clock_box.x / 2) - (home_score_box.width / 2);
			home_score_box.y = (float) hvlabel_y + (float) fontSize + (border * 4);
			// Draw home score box
			DrawRectangle (home_score_box.x - border, home_score_box.y - border, home_score_box.width + (border * 2), home_score_box.height + (border * 2), WHITE);
			DrawRectangle (home_score_box.x, home_score_box.y, home_score_box.width, home_score_box.height, BLACK);
			// Draw home score digits
			DrawDigit (score[HOME] / 100, home_score_box.x - (border * 3), home_score_box.y + border, border * 5, GOLD, 0);
			if (score[HOME] < 10)
				DrawDigit (-1, home_score_box.x + (border * 3.5f), home_score_box.y + border, border * 5, GOLD, 1);
			else
				DrawDigit ((score[HOME] % 100) / 10, home_score_box.x + (border * 3.5f), home_score_box.y + border, border * 5, GOLD, 1);
			DrawDigit (score[HOME] % 10, home_score_box.x + (border * 10), home_score_box.y + border, border * 5, GOLD, 1);

			// Visitor label
			DrawText
			(
				"VISITOR",
				(int) ((((screen_width - border) + (main_clock_box.x + main_clock_box.width + border)) / 2) - ((float) MeasureText ("VISITOR", fontSize) / 2)),
				hvlabel_y,
				fontSize,
				WHITE
			);
			// Update visitor score box
			visitor_score_box.width = border * 16;
			visitor_score_box.height = border * 11;
			visitor_score_box.x = (((screen_width - border) + (main_clock_box.x + main_clock_box.width + border)) / 2) - (visitor_score_box.width / 2);
			visitor_score_box.y = (float) hvlabel_y + (float) fontSize + (border * 4);
			// Draw visitor score box
			DrawRectangle (visitor_score_box.x - border, visitor_score_box.y - border, visitor_score_box.width + (border * 2), visitor_score_box.height + (border * 2), WHITE);
			DrawRectangle (visitor_score_box.x, visitor_score_box.y, visitor_score_box.width, visitor_score_box.height, BLACK);
			// Draw visitor score digits
			DrawDigit (score[VISITOR] / 100, visitor_score_box.x - (border * 3), visitor_score_box.y + border, border * 5, GOLD, 0);
			if (score[VISITOR] < 10)
				DrawDigit (-1, visitor_score_box.x + (border * 3.5f), visitor_score_box.y + border, border * 5, GOLD, 1);
			else
				DrawDigit ((score[VISITOR] % 100) / 10, visitor_score_box.x + (border * 3.5f), visitor_score_box.y + border, border * 5, GOLD, 1);
			DrawDigit (score[VISITOR] % 10, visitor_score_box.x + (border * 10), visitor_score_box.y + border, border * 5, GOLD, 1);
			//-------------------------------------------------------------------------------------


			// ### Foul displays
			//-------------------------------------------------------------------------------------
			// Update fouls label y value
			fouls_label_y = (int) (home_score_box.y + home_score_box.height + (border * 4));

			// Update home fouls box
			home_fouls_box.width = border * 10;
			home_fouls_box.height = border * 11;
			home_fouls_box.x = home_score_box.x;
			home_fouls_box.y = (float) fouls_label_y + (fontSize / 2) + (border * 2);
			// Home fouls label
			DrawText
			(
				"FOULS",
				(int) ((home_fouls_box.x + (home_fouls_box.width / 2)) - ((float) MeasureText ("FOULS", fontSize / 2) / 2)),
				fouls_label_y,
				fontSize / 2,
				WHITE
			);
			// Draw home fouls box
			DrawRectangle (home_fouls_box.x - border, home_fouls_box.y - border, home_fouls_box.width + (border * 2), home_fouls_box.height + (border * 2), WHITE);
			DrawRectangle (home_fouls_box.x, home_fouls_box.y, home_fouls_box.width, home_fouls_box.height, BLACK);
			// Draw home fouls digits
			if (fouls[HOME] < 10)
				DrawDigit (-1, home_fouls_box.x - (border * 3), home_fouls_box.y + border, border * 5, YELLOW, 0);
			else
				DrawDigit ((fouls[HOME] % 100) / 10, home_fouls_box.x - (border * 3), home_fouls_box.y + border, border * 5, YELLOW, 0);
			DrawDigit (fouls[HOME] % 10, home_fouls_box.x + (border * 3.5f), home_fouls_box.y + border, border * 5, YELLOW, 1);
			
			// Update visitor fouls box
			visitor_fouls_box.width = border * 10;
			visitor_fouls_box.height = border * 11;
			visitor_fouls_box.x = (visitor_score_box.x + visitor_score_box.width) - visitor_fouls_box.width;
			visitor_fouls_box.y = (float) fouls_label_y + (fontSize / 2) + (border * 2);
			// Home fouls label
			DrawText
			(
				"FOULS",
				(int) ((visitor_fouls_box.x + (visitor_fouls_box.width / 2)) - ((float) MeasureText ("FOULS", fontSize / 2) / 2)),
				fouls_label_y,
				fontSize / 2,
				WHITE
			);
			// Draw visitor fouls box
			DrawRectangle (visitor_fouls_box.x - border, visitor_fouls_box.y - border, visitor_fouls_box.width + (border * 2), visitor_fouls_box.height + (border * 2), WHITE);
			DrawRectangle (visitor_fouls_box.x, visitor_fouls_box.y, visitor_fouls_box.width, visitor_fouls_box.height, BLACK);
			// Draw visitor fouls digits
			if (fouls[VISITOR] < 10)
				DrawDigit (-1, visitor_fouls_box.x - (border * 3), visitor_fouls_box.y + border, border * 5, YELLOW, 0);
			else
				DrawDigit ((fouls[VISITOR] % 100) / 10, visitor_fouls_box.x - (border * 3), visitor_fouls_box.y + border, border * 5, YELLOW, 0);
			DrawDigit (fouls[VISITOR] % 10, visitor_fouls_box.x + (border * 3.5f), visitor_fouls_box.y + border, border * 5, YELLOW, 1);
			//-------------------------------------------------------------------------------------


			// ### TOL displays
			//-------------------------------------------------------------------------------------

			// Update home TOL box
			home_tol_box.width = border * 7;
			home_tol_box.height = border * 11;
			home_tol_box.x = home_fouls_box.x + home_fouls_box.width + (border * 5);
			home_tol_box.y = home_fouls_box.y;
			// Home TOL label
			DrawText
			(
				"T.O.L.",
				(int) ((home_tol_box.x + (home_tol_box.width / 2)) - ((float) MeasureText ("T.O.L.", fontSize / 2) / 2)),
				fouls_label_y,
				fontSize / 2,
				WHITE
			);
			// Draw home TOL box
			DrawRectangle (home_tol_box.x - border, home_tol_box.y - border, home_tol_box.width + (border * 2), home_tol_box.height + (border * 2), WHITE);
			DrawRectangle (home_tol_box.x, home_tol_box.y, home_tol_box.width, home_tol_box.height, BLACK);
			// Draw home TOL digit
			DrawDigit (tol[HOME], home_tol_box.x + border, home_tol_box.y + border, border * 5, YELLOW, 1);

			// Update visitor TOL box
			visitor_tol_box.width = border * 7;
			visitor_tol_box.height = border * 11;
			visitor_tol_box.x = visitor_fouls_box.x - visitor_tol_box.width - (border * 5);
			visitor_tol_box.y = visitor_fouls_box.y;
			// Home TOL label
			DrawText
			(
				"T.O.L.",
				(int) ((visitor_tol_box.x + (visitor_tol_box.width / 2)) - ((float) MeasureText ("T.O.L.", fontSize / 2) / 2)),
				fouls_label_y,
				fontSize / 2,
				WHITE
			);
			// Draw visitor TOL box
			DrawRectangle (visitor_tol_box.x - border, visitor_tol_box.y - border, visitor_tol_box.width + (border * 2), visitor_tol_box.height + (border * 2), WHITE);
			DrawRectangle (visitor_tol_box.x, visitor_tol_box.y, visitor_tol_box.width, visitor_tol_box.height, BLACK);
			// Draw visitor TOL digit
			DrawDigit (tol[VISITOR], visitor_tol_box.x + border, visitor_tol_box.y + border, border * 5, YELLOW, 1);

			//-------------------------------------------------------------------------------------

		EndDrawing ();

		//-----------------------------------------------------------------------------------------

	}
	//---------------------------------------------------------------------------------------------


	// # De-initialization
	//---------------------------------------------------------------------------------------------

	// Audio
	UnloadSound (buzzer_sound);
	CloseAudioDevice ();

	// Window icon
	UnloadImage (window_icon);

	// Window
	CloseWindow ();

	return EXIT_SUCCESS;
}

int TimeToInt (Time time)
{
	int int_time = 0;
	int_time += time.tenth_seconds;
	int_time += time.seconds * 10;
	int_time += time.ten_seconds * 100;
	int_time += time.minutes * 600;
	int_time += time.ten_minutes * 6000;
	return int_time;
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

