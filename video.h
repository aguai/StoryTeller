///////////////////////////////////////////////////////////////////////////////////////////////////
// Video class definition
// Author: Adam Streck
// Year 2009 - 2011
// 
// Class derived from SCENE. It is able to store and display subtitles.

#ifndef DEF_ENGINE_VIDEO
#define DEF_ENGINE_VIDEO
#include <dshow.h>
#include <vector>

class CMarkup;

class VIDEO : public SCENE {
private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inner class for subtitles
	// Holds a text, start and a stop time
	class SUBTITLE {
	private:
		friend class VIDEO;

		std::wstring text;
		LONGLONG time_start;
		LONGLONG time_end;

	public:	
		SUBTITLE(std::wstring sub_text, LONGLONG sub_start, LONGLONG sub_end);
		~SUBTITLE(){};
	};
	//
	///////////////////////////////////////////////////////////////////////////////////////////////

	// Subtitles holder
	std::vector<SUBTITLE> scene_subtitles;
	int active_subtitles;

public:
	VIDEO(const CMarkup*,  STATES*  state_pointer);
	~VIDEO();

	// Functions that cover subtitle creation and scheduling
	bool CreateSubtitles(void);
	bool ScheduleNextSubtitles(void);
	bool KillTimers(void);
	bool DrawSubtitles(bool display); // false clears the screen
	int VIDEO::GetActiveSub(void);
	int VIDEO::GetNextSub(void);

	// Derived interactivity functions
	bool LeftClick(const int x, const int y);
	bool RightClick(const int x, const int y);
	bool MouseMove(const int x, const int y);

	// Playback control
	bool Play(void);
	bool Pause(void);
	bool VideoEnded(void);
};
#endif