///////////////////////////////////////////////////////////////////////////////////////////////////
// Globals class definition
// Author: Adam Streck
// Year 2011
// 
// Globals is a holder for all the globally accessible variables and objects, as well as for a few
// system functions. It is only object that is shared globally.
// Class is mainly responsible for setup and it provides all the objects and variables to other classes.
// It also covers basic updating and resolution changes


#ifndef DEF_ENGINE_GLOBALS
#define DEF_ENGINE_GLOBALS

#include <Windows.h>
#include <string>

enum ENVIROMENTS { menu, game }; // Represents different interfaces the app can be in

class SETTING;
class AUDIO;

class GLOBALS {

private:
	friend class GAMEMENU;
	friend class GAMECONTROL;
	bool initialized;

	// System parts
	HWND window;        // Main window
	long start_time;    // Time at the start
	long run_time;      // Counts the time since the start of the game
	bool window_active; // Controlls activation / Deactivation of the whole process

	// Engine objects
	ENVIRONMENT* environment; // holds a location of a current environment
	GAMECONTROL* controller; 
	GAMEMENU*    main_menu;
	SETTING*     settings;
	AUDIO*       background_music;

	// graphics objects
	ULONG_PTR              gdiplus_token; // Used by GDIplus
	Gdiplus::Graphics*     SCR; // Screen graphics - draws to window
	Gdiplus::Graphics*     BMP; // Background buffer graphics - draws to memory_bitmap
	Gdiplus::Bitmap*       memory_bitmap; // Bitmap to draw on
	Gdiplus::CachedBitmap* cached_bitmap; // Cached version of the Bitmap

	// What is displayed before the output in log
	std::wstring prefix;

public:	
	GLOBALS();
	GLOBALS(const GLOBALS & original) { throw (L"GLOBALS copy constructor used"); }
	GLOBALS operator=(const GLOBALS & original) { throw (L"GLOBALS assign operator used"); }
	~GLOBALS();

	// SetUp functions
	void SetupEngine(void);
	bool SetUpGraphics(void);
	bool SetUpMachine(void); 
	void SetWindow(HWND new_window) { window = new_window; }
	bool SetUpSettings(void);
	bool SetUpBackgrounAudio(void);

	// Get-ers
	// DO NOT DELETE Get-ers pointers
	bool const             GetActive(void) { return window_active; }
	long const             GetTime(void) { return static_cast<long>(GetTickCount()) - start_time; }
	HWND                   GetWindow(void) { return window; }
	ENVIRONMENT*           GetEnvironment(void) {return environment;}
	GAMECONTROL*           GetController(void) { return controller; }
	GAMEMENU*			   GetMainMenu(void) { return main_menu; }
	SETTING*               GetGameSettings(void) { return settings; }
	Gdiplus::Graphics*     GetScreenObject(void) { return SCR; }
	Gdiplus::Graphics*     GetCanvasObject(void) { return BMP; }
	Gdiplus::Bitmap*       GetDrawBitmap(void) { return memory_bitmap; }
	Gdiplus::CachedBitmap* GetCachedBitmap(void) { return cached_bitmap; }

	// Set-ers
	ENVIRONMENT* SetEnvironment(ENVIROMENTS env_type);
	inline void  SetActive(bool is_active) { window_active = is_active; }

	// Work functions
	void ReCacheBitmap(void);
	bool ChangeDisplayMode(int pixel_width, int pixel_height, bool fullscreen);
	bool RewindAudio(void);
};


#endif