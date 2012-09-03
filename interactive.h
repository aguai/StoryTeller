///////////////////////////////////////////////////////////////////////////////////////////////////
// Video class definition
// Author: Adam Streck
// Year 2009 - 2011
// 
// Class derived from SCENE. Interactive class mainly holds and manages objects.

#ifndef DEF_ENGINE_INTERACTIVE
#define DEF_ENGINE_INTERACTIVE

#include <dshow.h>
#include <D3D9.h>
#include <Vmr9.h>
#include <windows.h>
#include <gdiplus.h>
#include <map>
#include <string>
#include <vector>

class CMarkup;
class SCENE;
class STATES;

class INTERACTIVE : public SCENE {
private:

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inner class for objects
	// Holds:
	//    object position in a form of polygon region
	//    name that displays when the region is under the cursor
	//    caption displayed and playbacked when the object is right-clicked
	//    playback for the left click
	//    changes that are caused by left click 
	class SCENE_OBJECT {
	private:
		friend class INTERACTIVE; // Superior class has access to private members (behaves as a structure)

		Gdiplus::Region * position;
		std::wstring name;
		std::wstring caption;
		std::wstring sound_path;
		std::wstring caption_sound_path;
		std::vector<state_changes> caused_changes; 
		bool finish_scene;

	public:	
		SCENE_OBJECT(const Gdiplus::GraphicsPath* boundaries, const std::wstring object_name, const std::wstring object_caption, 
					 const std::wstring object_sound_path, const std::wstring object_caption_sound_path, const std::vector<state_changes> & object_state_changes, const bool finisher);
		SCENE_OBJECT(const SCENE_OBJECT&);
		SCENE_OBJECT& operator=(const SCENE_OBJECT&) {throw "SCENE_OBJECT equality operator called"; }
		~SCENE_OBJECT();
	};
	//
	///////////////////////////////////////////////////////////////////////////////////////////////

	// objects system
	std::vector<SCENE_OBJECT> objects;
	int last_active_object; // Information for object name redisplay
	bool caption_displayed; // Switch between displaying the name or the caption of the active object

public:

	INTERACTIVE(const CMarkup*, STATES*  state_pointer);
	INTERACTIVE(const INTERACTIVE &) { throw (L"INTERACTIVE copy contuctor has been called"); }
	INTERACTIVE& operator=(const INTERACTIVE &) { throw (L"INTERACTIVE equality operator has been called"); }
	~INTERACTIVE();

	// Object control functions
	bool CreateObjects(); // function creates objects from script - first it take names and than it creates path for a region and afterwards the map of state changes
	void DrawString(const Gdiplus::PointF & hit_position, const std::wstring & text);
	int  DetectCollision(const Gdiplus::PointF & hit_position);

	// Derived interactivity functions
	bool LeftClick(const int x, const int y);
	bool RightClick(const int x, const int y);
	bool MouseMove(const int x, const int y);

	// Playback control
	bool VideoEnded( void );
};
#endif