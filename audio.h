///////////////////////////////////////////////////////////////////////////////////////////////////
// Audio class definition
// Author: Adam Streck
// Year 2011
// 
// Class holds all the necessities for basic audio playback and volume control.

#ifndef DEF_ENGINE_AUDIO
#define DEF_ENGINE_AUDIO

#include <dshow.h>
#include <string>

class AUDIO{

private:

	// system control
	HRESULT       hr;	
	std::wstring  fail_msg;
	bool          audio_built;
	OAFilterState filter_state;

	// DirectShow Interfaces
	IGraphBuilder *audio_graph;
	IMediaControl *audio_media_control;
	IMediaEvent   *audio_media_event;
	IMediaSeeking *audio_media_seeking;
	IBasicAudio   *audio_volume_control;

	// Info variabless
	LONGLONG time_passed;
	LONGLONG stop_time;

public:	

	AUDIO();
	AUDIO(const AUDIO & original) { throw (L"AUDIO copy constructor");  }
	AUDIO operator=(const AUDIO & original) { throw (L"AUDIO assign operator used"); }
	~AUDIO();

	// Creation
	bool Build(void);
	bool RenderNewFile(std::wstring source, int volume);

	// Playback control
	bool Play(void);
	bool Pause(void);
	bool Stop(void);
	bool Rewind(void);
	bool SetVolume(int percent);
	bool SetEndTimer(const int TIMER_NAME) ;
};

#endif