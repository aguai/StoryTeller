///////////////////////////////////////////////////////////////////////////////////////////////////
// Globals class functions
// Author: Adam Streck
// Year 2011
// 
// Globals is a holder for all the globally accessible variables and objects, as well as for a few
// system functions. It is only object that is shared globally.
// Class is mainly responsible for setup and it provides all the objects and variables to other classes.
// It also covers basic updating and resolution changes

// TODO: Fix change display mode
// TODO: React on playback end

#include "header.h"

/**
 * Just nullifies + starts counting the time
 */
GLOBALS::GLOBALS() {
	initialized = false;

	window_active = true;
	start_time    = static_cast<long>(GetTickCount());
	run_time      = 0;

	environment      = nullptr;
	controller       = nullptr;
	main_menu        = nullptr;
	window           = nullptr;
	settings         = nullptr;
	background_music = nullptr;

	SCR = nullptr;
	BMP = nullptr; // Background buffer graphics - draws to memory_bitmap
	memory_bitmap = nullptr;
	cached_bitmap = nullptr;
}

/**
 * Deletes objects, shuts down Gdiplus and COM and restores resolution
 */
GLOBALS::~GLOBALS() {
	if (initialized) {
		delete SCR;
		delete memory_bitmap;
		delete BMP;
		delete cached_bitmap; 
		delete controller;
		delete main_menu;
		CoUninitialize();
		Gdiplus::GdiplusShutdown(gdiplus_token);
	}
	Log(L"DestroyWindow", DONT_TERM * API_FUNC * LOG_FAIL, !DestroyWindow(window));
	Log(L"Restoring the resolution in the end.", DONT_TERM * MY_FUNC * LOG_FAIL, !ChangeDisplayMode(window_width, window_height, false));	
}

/**
 * Sets the environment pointer so it points to the current environment and handles the system calls.
 */
ENVIRONMENT* GLOBALS::SetEnvironment(ENVIROMENTS env_type) { 
	ENVIRONMENT * temp = environment; 
	switch (env_type) {
		case (menu):
			environment = main_menu;
		break;

		case (game):
			environment = controller;
		break;
	};
	return temp; 
}

/**
 * Basic setup.
 */
void GLOBALS::SetupEngine(void) {
	if(!SetUpGraphics()) {
		Log(L"Screen graphics allocation failed.", TERMINATE * LOG_FAIL, 1); 
		return;
	}
	if(!SetUpMachine()){
		Log(L"Engine objects allocation failed.", TERMINATE * LOG_FAIL, 1); 
		return;
	}
	Log(L"Background audio creation and failed.", DONT_TERM * LOG_FAIL, !SetUpBackgrounAudio()); 

	SetEnvironment(menu);

	initialized = true;
	Log(L"Engine initialized.", DONT_TERM * LOG_ALL, !initialized); 
}

/**
 * Graphics setup
 */
bool GLOBALS::SetUpGraphics(void) {
	// Initialization of GdiPlus and COM systems
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::Status return_val = GdiplusStartup(&gdiplus_token, &gdiplusStartupInput, NULL);
		Log(L"GdiplusStartup(&gdiplus_token, &gdiplusStartupInput, NULL)", GDI_FUNC * TERMINATE, return_val);
			if(return_val) return false;
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		Log(L"CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);", DX_FUNC * TERMINATE, hr); 
			if(hr) {Gdiplus::GdiplusShutdown(gdiplus_token); return false;}

	// Creation of graphics objects
	SCR = new Gdiplus::Graphics(window);
		if(SCR == nullptr) 
			{ Log(L"Screen graphics object allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1);  Gdiplus::GdiplusShutdown(gdiplus_token); return false;}
	memory_bitmap = new Gdiplus::Bitmap(settings->GetValue<int>(L"resolution_x"), settings->GetValue<int>(L"resolution_y"));
		if(memory_bitmap == nullptr) 
			{ Log(L"Bitmap object allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1); Gdiplus::GdiplusShutdown(gdiplus_token); delete SCR; return false;}
	BMP = new Gdiplus::Graphics(memory_bitmap);
		if(BMP == nullptr) 
			{ Log(L"Bitmap graphics object allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1); Gdiplus::GdiplusShutdown(gdiplus_token); delete memory_bitmap; delete SCR; return false;}
	cached_bitmap = new Gdiplus::CachedBitmap(memory_bitmap, SCR);
		if(cached_bitmap == nullptr)
			{ Log(L"Cached bitmap object allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1); Gdiplus::GdiplusShutdown(gdiplus_token); delete BMP; delete memory_bitmap; delete SCR; return false;}

	return true;
}

/**
 * Engine objects allocation.
 */
bool GLOBALS::SetUpMachine(void) {
	main_menu = new GAMEMENU();
		if(main_menu == nullptr)
			{ Log(L"Main menu allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1); return false; }
	controller = new GAMECONTROL();
		if(controller == nullptr)
			{ Log(L"GameControl allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1); delete main_menu; return false; }

	return true;
}

/**
 * Creates and starts the music on the background.
 */ 
bool GLOBALS::SetUpBackgrounAudio(void) {
	background_music = new AUDIO();
	if (background_music == nullptr) {
		Log(L"Scene_audio object failed.", DONT_TERM * LOG_FAIL, true);
		return false;
	} 

	bool result = background_music->Build();
	Log(L"background_music->Build()", MY_FUNC * LOG_FAIL * DONT_TERM, !result);
	if (result == false)
		return false;

	std::wstring bg_path = settings->GetValue<std::wstring>(L"audios_path") + settings->GetValue<std::wstring>(L"bg_music") + settings->GetValue<std::wstring>(L"audios_suffix");
	if (!background_music->RenderNewFile(bg_path, settings->GetValue<int>(L"music_volume")))
		return false;
	if(!background_music->Play())
		return false;
	if(!background_music->SetEndTimer(MUSIC_PLAYBACK))
		return false;
	return true;
}

/**
 * Restarts the background audio after finishing
 */
bool GLOBALS::RewindAudio(void) {
	if (!background_music->Rewind())
		return false;
	if(!background_music->SetEndTimer(MUSIC_PLAYBACK))
		return false;
	return true;
}


/**
 * Settings loadup.
 */
bool GLOBALS::SetUpSettings(void) {
	settings = new SETTING();
		if(settings == nullptr)
			{ Log(L"SETTING object allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1); return false; }
	Log(L"Setup of Settings failed.", MY_FUNC * TERMINATE, !settings->LoadSettings());

	return true;
}

/**
 * Re-caching of the bitmap when redrawing
 */
void GLOBALS::ReCacheBitmap(void) {
	delete cached_bitmap;
	cached_bitmap = nullptr;
	cached_bitmap = new Gdiplus::CachedBitmap(memory_bitmap, SCR);
		if(cached_bitmap == nullptr)
			Log(L"Cached bitmap object allocation failed, probably out of memory", TERMINATE * LOG_FAIL, 1); 
}

/**
 * ChangeDisplayMode handles the change of the resolution of the screen. 
 */
bool GLOBALS::ChangeDisplayMode(int pixel_width, int pixel_height, bool is_fullscreen) {
	/* For now it terminates application, when unable to change the resolution*/
	if(!is_fullscreen) { 
		ChangeDisplaySettings(NULL,0); // Switch to registered resolution

		// Change properties of the window
		int win_border_width  = window_width  + (GetSystemMetrics(SM_CXFIXEDFRAME))*2;
		int win_border_height = window_height + (GetSystemMetrics(SM_CYCAPTION)) + (GetSystemMetrics(SM_CYFIXEDFRAME))*2;
		SetWindowLongPtr(window, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		SetWindowPos(window, HWND_TOP, (GetSystemMetrics(SM_CXSCREEN) - win_border_width)/2, (GetSystemMetrics(SM_CYSCREEN) - win_border_height)/2, win_border_width, win_border_height, SWP_SHOWWINDOW);
		return true;
	}
	else {		
		DEVMODE dev_mode = {0};

		Log(L"EnumDisplaySettings", API_FUNC * DONT_TERM, EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dev_mode));

		dev_mode.dmPelsWidth = pixel_width;
		dev_mode.dmPelsHeight = pixel_height;
		dev_mode.dmBitsPerPel = 32;
		dev_mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;  // change of height and width

		LONG result = ChangeDisplaySettings(&dev_mode,CDS_FULLSCREEN);


		/*if(result){ // won't work with 32 bits, trying 24
			dev_mode.dmBitsPerPel = 24;
			result = ChangeDisplaySettings(&dev_mode,CDS_FULLSCREEN);
		}
		if(result){ // won't work with 24 bits, trying 16
			dev_mode.dmBitsPerPel = 16;
			result = ChangeDisplaySettings(&dev_mode,CDS_FULLSCREEN);
		}
		if(result){ // Won't work at all in fullscreen -> error log
			std::wstring fail_msg(L"Application failed to change the screen resolution (your device probably doesn't support ");
			fail_msg.append(boost::lexical_cast<std::wstring, int>(pixel_width));
			fail_msg.append(L"x");
			fail_msg.append(boost::lexical_cast<std::wstring, int>(pixel_height));
			fail_msg.append(L") application will continue in windowed mode.");
			int user_response = MessageBox(NULL, fail_msg.c_str(), L"Resolution change failure", MB_OK);
			GetGameSettings()->SetValue<bool>(L"fullscreen", false);
			return false;
		}*/

		// Change properties of the window
		SetWindowLongPtr(window, GWL_STYLE, WS_POPUP);
		SetWindowPos(window, HWND_TOP, 0, 0, window_width, window_height, SWP_SHOWWINDOW);
		ShowWindow(window, SW_SHOW);
		return true;
	}
}
