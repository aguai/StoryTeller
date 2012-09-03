///////////////////////////////////////////////////////////////////////////////////////////////////
// Audio class
// Author: Adam Streck
// Year 2011
// 
// Functions for building audio graphs, rendering sound, playback and volume control.

// TODO : Fix play, pause, set_volume etc...
// Add stop function
// Add timer control

#include "header.h"
extern GLOBALS* globals;

AUDIO::AUDIO() {
	hr = E_FAIL;	
	fail_msg = L"uninitialized fail_msg";
	audio_built = false;
}

AUDIO::~AUDIO() {
	if(audio_built){
		audio_graph->Release(); 
		audio_media_event->Release();
		audio_media_control->Release();
		audio_volume_control->Release();
		audio_media_seeking->Release();
	}
}

/** 
 * Builds all the necessary objects - should be called only once per object.
 */
bool AUDIO::Build(){
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&audio_graph);
		if(hr != S_OK) { fail_msg = L"CoCreateInstance - IGraphBuilder (audio)"; return false; }	
	hr = audio_graph->QueryInterface(IID_IMediaControl, (void **)&audio_media_control);
		if(hr != S_OK) { fail_msg = L"QueryInterface - IMediaControl (video)"; audio_graph->Release(); return false; }
	hr = audio_graph->QueryInterface(IID_IMediaEvent, (void**)&audio_media_event);
		if(hr != S_OK) { fail_msg = L"QueryInterface - IMediaEvent"; audio_graph->Release(); audio_media_control->Release(); return false; }
    hr = audio_graph->QueryInterface(IID_IBasicAudio, (void**)&audio_volume_control);
		if(hr != S_OK) { fail_msg = L"QueryInterface - IBasicAudio (video)"; audio_graph->Release(); audio_media_control->Release(); audio_media_event->Release(); return false; }
	hr = audio_graph->QueryInterface(IID_IMediaSeeking, (void**)&audio_media_seeking);
		if(hr != S_OK) { fail_msg = L"QueryInterface - IBasicAudio (video)"; audio_graph->Release(); audio_media_control->Release(); audio_media_event->Release(); audio_volume_control->Release(); return false; }
	return true; 
}

/**
 * Playback start
 */
bool AUDIO::Play() {
	audio_media_control->Run();

	// Wait until the transition is complete or 500 ms
	int loop_count = 0;
	do {
		hr = audio_media_control->GetState(50, &filter_state);
		Sleep(50);
		loop_count++;
	} while (hr == VFW_S_STATE_INTERMEDIATE && loop_count < 10);
	Log(L"audio_media_control->GetState - Audio play", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);

	if (filter_state != State_Running) {
		Log(L"Audio is not running.", LOG_ALL * TAB * TERMINATE, filter_state);
		return false;
	}

	return true;
}

/**
 * Playback pause
 */
bool AUDIO::Pause() {
	audio_media_control->Pause();

	// Wait until the transition is complete or 500 ms
	int loop_count = 0;
	do {
		hr = audio_media_control->GetState(50, &filter_state);
		Sleep(50);
		loop_count++;
	} while (hr == VFW_S_STATE_INTERMEDIATE && loop_count < 10);
	Log(L"audio_media_control->GetState - Audio pause", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);

	if (filter_state != State_Paused) {
		Log(L"Audio was not paused.", LOG_ALL * TAB * TERMINATE, filter_state);
		return false;
	}

	return true;
}

/**
 * Playback stop
 */
bool AUDIO::Stop() {
	audio_media_control->Stop();

	// Wait until the transition is complete or 500 ms
	int loop_count = 0;
	do {
		hr = audio_media_control->GetState(50, &filter_state);
		Sleep(50);
		loop_count++;
	} while (hr == VFW_S_STATE_INTERMEDIATE && loop_count < 10);
	Log(L"audio_media_control->GetState - Audio stop", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);

	if (filter_state != State_Stopped) {
		Log(L"Audio was not stopped.", LOG_ALL * TAB * TERMINATE, filter_state);
		return false;
	}

	return true;
}

/**
 * Rewinds the stream to the begining
 */
bool AUDIO::Rewind() {
	Log(L"Rewind audio Stop()", MY_FUNC, !Stop());

	// Set new positions - start at 0 and don't change end
	LONGLONG null = 0;
	hr = audio_media_seeking->SetPositions(&null, AM_SEEKING_AbsolutePositioning, &null, AM_SEEKING_NoPositioning);
	Log(L"audio_media_seeking->SetPositions(&null, AM_SEEKING_AbsolutePositioning, &null, AM_SEEKING_NoPositioning);", DX_FUNC * TAB, hr); 

	Log(L"Rewind audio Play()", MY_FUNC, !Play());

	return !hr;
}

/** 
 * Volume setup
 */
bool AUDIO::SetVolume(int percent) {
	int volume = (percent - 100)*100;

	hr = audio_volume_control->put_Volume(volume); 
		HRLog(L"IBasicAudio.put_Volume (audio)", hr, false);
	return true;
}

/** 
 * Rendering an input file
 */
bool AUDIO::RenderNewFile(std::wstring source, int volume) {
	Log(L"Stop audio playback befor rendering new file", MY_FUNC * TAB, !Stop());

	// Obtain currently emplyed filters, if there are any and remove them, aka stop current playback
	// It because it is used eve if there are no filters at all, it doesn't log
	IEnumFilters * filters;
	IBaseFilter  * filter;
	ULONG          obtained;
	Log(L"audio_graph->EnumFilters(&filters)", DX_FUNC * TAB * TAB, audio_graph->EnumFilters(&filters));
	Log(L"filters->Next(1, &filter, &obtained)", DX_FUNC * TAB * TAB * DONT_LOG, filters->Next(1, &filter, &obtained));
	filters->Release();
	// If there was a filter, remove it
	if (obtained > 0) {
		Log(L"audio_graph->RemoveFilter(filter)", DX_FUNC * TAB * TAB, audio_graph->RemoveFilter(filter));
		filter->Release();
	}
	
	// Render new file
	hr = audio_graph->RenderFile(source.c_str(), NULL);
	if (hr != S_OK) {
		Log(L"IGraphBuilder.RenderFile() (audio) ", DX_FUNC * TAB, hr);
		return false;
	}

	Log(L"Set audio volume after rendering new file", MY_FUNC * TAB,! SetVolume(volume));

	Log(L"Play audio after rendering new file", MY_FUNC * TAB, !Play());

	return true;
}

/**
 * Starts the timer for the end of the playback
 */
bool AUDIO::SetEndTimer(const int TIMER_NAME) {
	// Try to set the timer - twice
	Log(L"Audio seeker GetPositions(&time_passed,&stop_time) at Play()", DX_FUNC * TAB, audio_media_seeking->GetPositions(&time_passed,&stop_time));
	if (!SetTimer(globals->GetWindow(), MUSIC_PLAYBACK, static_cast<UINT>((stop_time - time_passed) / 10000), (TIMERPROC) NULL)) {
		Log(L"SetTimer(globals->GetWindow(), MUSIC_PLAYBACK,...)", TAB * LOG_FAIL * DONT_TERM * API_FUNC, 1);
		if (!SetTimer(globals->GetWindow(), MUSIC_PLAYBACK, static_cast<UINT>((stop_time - time_passed) / 10000), (TIMERPROC) NULL)) {
			Log(L"SetTimer(globals->GetWindow(), MUSIC_PLAYBACK,...)", TAB * LOG_FAIL * DONT_TERM * API_FUNC, 1);	
			return false;
		}
	}
	return true;
}