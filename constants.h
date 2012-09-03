#ifndef DEF_ENGINE_VARIABLES
#define DEF_ENGINE_VARIABLES

#include <windows.h>
#include <gdiplus.h>
#include <string>

// Setting file
const std::wstring setting_file_name(L"setting.stx");

// constants for the message loop
#define WM_GRAPHNOTIFY WM_APP + 1  // used for registering Graphnotify message
#define SCENE_FINISHED 0x8002
#define SCENE_CREATED  0x8003
#define GAME_FINISHED  0x8004

// timers 
const int VIDEO_PLAYBACK  = 1;
const int SOUND_PLAYBACK  = 2;
const int MUSIC_PLAYBACK  = 3;
const int SUBTITLES_END   = 4;
const int SUBTITLES_START = 5;

// MACROS
typedef std::pair<std::wstring, int> STATE ;         // Game system states
typedef std::pair<std::wstring, std::wstring> SETVAL; // Setting values 

// structure throught which game states are changed
struct state_changes {std::wstring name; int value; bool replace;};	

// window coordinates
const int window_width  = 1280;
const int window_height = 720;
const int upper_left_corner_X = 0; // used for button positioning 
const int upper_left_corner_Y = 0; //

// properties for the GDI+ Graphics
const Gdiplus::Color key_color(0,0,0); // color of the background - will be removed
const Gdiplus::Color text_surrounding_color(32,32,32);
const Gdiplus::Color text_fill_color(255,255,255);
const Gdiplus::Color nonactive_fill_color(159,159,159);
const Gdiplus::REAL  text_size = 24;
const Gdiplus::REAL  text_stroke_width = 3;
const std::wstring   my_font_family(L"Arial");

#endif
