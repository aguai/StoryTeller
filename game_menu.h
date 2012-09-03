#ifndef DEF_ENGINE_GAMEMENU
#define DEF_ENGINE_GAMEMENU

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include "environment.h"

class GAMEMENU : public ENVIRONMENT {
private:
	// Data types
	// enum screens {main, options, credits, endings};
	// ABSTRACT MENU_ITEM class
	class MENU_ITEM {
	public:
		friend class GAMEMENU;
		enum picture_enumeration {basic, highlited, activated};
		picture_enumeration active_picture;
		Gdiplus::Rect position;
		int ID;	
		virtual void Draw() = 0;
	};
	// BUTTON - on/off menu button
	class BUTTON : public MENU_ITEM {
	private:
		Gdiplus::Image * picture_basic;
		Gdiplus::Image * picture_highlited;
		Gdiplus::Image * picture_activated;	
	public:	
		BUTTON(const BUTTON &);
		BUTTON & operator=(const BUTTON & image);
		BUTTON(const int x, const int y, const wchar_t * path_basic, const wchar_t * path_highlited, const wchar_t * path_activated, int button_ID);
		~BUTTON();
		virtual void Draw();
	};
	// SLIDER - fuzzy menu button
	class SLIDER : public MENU_ITEM  {
	private:
		Gdiplus::Image * picture_basic;
		Gdiplus::Image * picture_highlited;
		Gdiplus::Image * picture_slider;
		int value;
	public:	
		int slider_width;
		SLIDER(const SLIDER &);
		SLIDER & operator=(const SLIDER & image);
		SLIDER(const int x, const int y, const wchar_t * path_basic, const wchar_t * path_highlited, const wchar_t * path_slider, int button_ID, int input_value);
		~SLIDER();
		void SetValue(int new_value) {value = new_value;}
		virtual void Draw();
	};

	// variables
	bool menu_active;
	std::vector<MENU_ITEM*> menu_items;
	Gdiplus::Image * background_picture;

public:
	GAMEMENU();
	~GAMEMENU();
	bool MouseMove(const int x, const int y);
	bool LeftClick(const int x, const int y);
	bool RightClick(const int x, const int y);
	bool Redraw();  // Drawing to bitmap
	bool Display(); // Displaying the bitmap
	bool GameStart();
	bool ExitGame();
	bool Continue();
	int ChangeVolume(int pos, int width);
	bool IsMenuActive() {return menu_active;}
	void ActivateMenu() {menu_active = true;}
	void DisableMenu() {menu_active = false;}
	void Fullscreen();
};

#endif