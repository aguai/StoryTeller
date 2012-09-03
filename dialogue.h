///////////////////////////////////////////////////////////////////////////////////////////////////
// Dialogue class definition
// Author: Adam Streck
// Year 2009 - 2011
// 
// Class derived from SCENE. It is able to create and store interactive dialogue options.

#ifndef DEF_ENGINE_DIALOGUE
#define DEF_ENGINE_DIALOGUE

#include <map>
#include <string>
#include <vector>
#include "scene.h"

class CMarkup;
class STATES;

class DIALOGUE : public SCENE {
private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inner class for conversation options
	// Holds:
	//    path of the text
	//    content of the text
	//    changes that the option causes
	class OPTION {
	private:
		friend class DIALOGUE;// Superior class has access to private members (behaves as a structure)

		Gdiplus::GraphicsPath * text_path;
		Gdiplus::Rect * boundaries;
		std::wstring text;
		std::vector<state_changes> caused_changes; 
	public:		
		OPTION(std::wstring & object_text, Gdiplus::GraphicsPath * object_path, std::vector<state_changes> & object_state_changes);
		OPTION(const OPTION& original);
		OPTION& operator=(const OPTION &) { throw (L"OPTION equality operator has been called."); }
		~OPTION();
	};
	//
	///////////////////////////////////////////////////////////////////////////////////////////////

	// Properties of the options
	std::vector<OPTION> options;
	int line_bot;    // top position of the bottom option line
	int line_height; // height of the single line
	int highlited_option;
	int left_space;  // Pixels preceeding the text from left.

public:

	DIALOGUE(const CMarkup*, STATES*  state_pointer);
	DIALOGUE(const DIALOGUE &) { throw (L"DIALOGUE copy contuctor has been called."); }
	DIALOGUE& operator=(const DIALOGUE &) { throw (L"DIALOGUE equality operator has been called."); }
	~DIALOGUE();

	// Options control functions
	bool CreateOptions();
	bool DrawOptions();
	bool CreateOptionsGraphics(const std::wstring & text, Gdiplus::GraphicsPath & text_path);
	int  DetectCollision(const Gdiplus::Point & hit_position);

	// Derived interactivity functions
	bool LeftClick(const int x, const int y);
	bool RightClick(const int x, const int y);
	bool MouseMove(const int x, const int y);

	// Playback control
	bool VideoEnded( void );
};
#endif