///////////////////////////////////////////////////////////////////////////////////////////////////
// StoryTeller engine v. 1.0
// Author: Adam Streck
// Year 2009-2011
// 
// Conventions:
//    DATA_TYPES
//    variable_names
//    constant_names
//    FunctionNames

// REMINDERS:
// Remove scene created posts

#include "header.h"
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

// Engine holder object
GLOBALS* globals = nullptr;

/* Window procedure which handles system messages. */
LRESULT CALLBACK WindowProcedure( HWND window, UINT zprava, WPARAM w_param,  LPARAM l_param )
{
	switch (zprava)
	{
		case WM_ACTIVATE: // On window activation / Deactivation - stops the game, if nescessary
			              // Clicking outside of window (task-bar) doesn't necesarrily stop reading of the messages
			if (w_param != WA_INACTIVE) {
				if (globals->GetGameSettings()->GetValue<bool>(L"fullscreen"))
					Log(L"after-deactivation resolution change", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->ChangeDisplayMode(window_width, window_height, true));
				globals->SetActive(true);
				if(typeid(*globals->GetEnvironment()) == typeid(GAMECONTROL))
					Log(L"after-activation controller->Play()", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetController()->Play());
			}
			else {
				globals->SetActive(false);
				if (globals->GetGameSettings()->GetValue<bool>(L"fullscreen"))
					Log(L"after-deactivation resolution change", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->ChangeDisplayMode(window_width, window_height, false));
				if(typeid(*globals->GetEnvironment()) == typeid(GAMECONTROL))
					Log(L"after-deactivation controller->Pause()", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetController()->Pause());
			}
		break;

		case WM_CLOSE: // On close message
			PostQuitMessage(0);
		break;

		case WM_PAINT: // When repaint is needed
			Log(L"WM_PAINT ValidateRect(window, NULL)", STARS * DONT_TERM * API_FUNC * LOG_FAIL, !ValidateRect(window, NULL));
			Log(L"WM_PAINT environment->Display()", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetEnvironment()->Display());
		break;

		case WM_MOUSEMOVE:
			globals->GetEnvironment()->MouseMove(MAKEPOINTS(l_param).x, MAKEPOINTS(l_param).y);
		break;

		case WM_LBUTTONDOWN:
			if (globals->GetActive() != false) // ?
				globals->GetEnvironment()->LeftClick(MAKEPOINTS(l_param).x, MAKEPOINTS(l_param).y);
		break;

		case WM_RBUTTONDOWN:
			globals->GetEnvironment()->RightClick(MAKEPOINTS(l_param).x, MAKEPOINTS(l_param).y);
		break;

		case WM_KEYDOWN:
			switch (w_param){
				case VK_ESCAPE: // Escape goes into and from the menu
					if (typeid(*globals->GetEnvironment()) == typeid(GAMEMENU)){ // Out from menu
						Log(L"after-escape scene controller->Display()", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetController()->Display());
						Log(L"after-escape scene controller->Play()", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetController()->Play());
						globals->SetEnvironment(game);
					}
					else { // Into the menu
						Log(L"after-escape scene controller->Pause()", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetController()->Pause());
						globals->SetEnvironment(menu);
						Log(L"after-escape main_menu->Display()", STARS * DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetMainMenu()->Display());
					}
				break;
			}
		break;

		case WM_TIMER:
			switch (w_param) 
			{ 
				case VIDEO_PLAYBACK: 
					Log(L"VideoEnded()", MY_FUNC * TAB, globals->GetController()->GetScene()->VideoEnded());
				break; 
 
				case MUSIC_PLAYBACK: 
					Log(L"RewindAudio() - background music", MY_FUNC, globals->RewindAudio());
				break;

				case SUBTITLES_START:
					Log(L"SubtitlesStart()", MY_FUNC, !globals->GetController()->SubtitlesStart());
				break;

				case SUBTITLES_END:
					Log(L"SubtitlesEnd()", MY_FUNC, !globals->GetController()->SubtitlesEnd());
				break;
			} 
		break;

		case WM_GRAPHNOTIFY: // video event
			if(globals->GetController()->IsSceneAccesible())
				globals->GetController()->VideoEventOccured();
		break;

		case SCENE_CREATED: // New scene was created
			globals->GetController()->SceneAccesible(true);
		break;

		case SCENE_FINISHED: // Last scene ended
			globals->GetController()->SceneAccesible(false);
			if(!globals->GetController()->DidGameFinished())
				Log(L"controller->CreateNewScene()", TERMINATE * MY_FUNC * LOG_FAIL, !globals->GetController()->CreateNewScene());
			else {
				ShowCursor(true);
				globals->SetEnvironment(menu);
				Log(L"after game finished main_menu->Display()",  DONT_TERM * MY_FUNC * LOG_FAIL, !globals->GetMainMenu()->Display());				
			}
		break;

		default:
			return (DefWindowProc(window, zprava, w_param, l_param));
	}
	return 0;
}

/**
 * Creates blank window of zero size
 */
void CreateMyWindow(HINSTANCE & instance){
	HWND       window;
	WNDCLASSEX WC;

	WC.cbSize =        sizeof(WNDCLASSEX);
	WC.style =         0; 
	WC.lpfnWndProc =   (WNDPROC)WindowProcedure;
	WC.cbClsExtra =    0;
	WC.cbWndExtra =    0;
	WC.hInstance =     instance;
	WC.hIcon =         LoadIcon(instance, MAKEINTRESOURCE(101));
	WC.hIconSm =       LoadIcon(instance, MAKEINTRESOURCE(101));
	WC.hCursor =       LoadCursor(NULL, IDC_ARROW);
	WC.hbrBackground = (HBRUSH) (0x00000000);
	WC.lpszMenuName =  NULL;
	WC.lpszClassName = L"my_class";

	Log(L"RegisterClassEx(...)", TERMINATE * MY_FUNC * LOG_FAIL, !RegisterClassEx(&WC));

	// Creates crippled window - it is resized with resolution change
	window = CreateWindowEx(0, L"my_class", L"StoryTeller engine v1.0", 0, 0, 0, 0, 0, NULL, NULL, instance, NULL ); 
	Log(L"CreateWindowEx(...)", TERMINATE * MY_FUNC * LOG_FAIL, !window);
	globals->SetWindow(window);
}

/** 
 * Main function - creates a window and starts the message loop. 
 */
INT WINAPI WinMain(HINSTANCE instance, HINSTANCE predchozi_instance, LPSTR sp_prikazovy_radek, INT i_prikaz_zobrazeni ) { 
	// Loads global values
	globals = new GLOBALS();
		if(globals == nullptr)
			Log(L"Globals object allocation failed, probably out of memory.", TERMINATE * LOG_ALL, true);
	globals->SetUpSettings();

	// Sets things up
	Log(L"Engine initialization started.", DONT_TERM * LOG_ALL, false);
	CreateMyWindow(instance); 
	Log(L"Startup resolution change", DONT_TERM * MY_FUNC * LOG_FAIL, !globals->ChangeDisplayMode(window_width, window_height, globals->GetGameSettings()->GetValue<bool>(L"fullscreen"))); // Resolution must be set before Gdiplus startup
	globals->SetupEngine();
    // ShowWindow(globals->GetWindow(), SW_SHOW); // Activates and displays window

	// Message pump - Update is sent at a random time - no FPS counter employed
    MSG msg;
    DWORD last_tick = 0;
	while( true )
	{
		GetMessage(&msg,(HWND) NULL, 0, 0);
		// Log(msg.message); // When the log is needed
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT){ 
			delete globals;
			break;
		}
    }

	return static_cast<int>(msg.wParam);
}          