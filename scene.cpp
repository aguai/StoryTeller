///////////////////////////////////////////////////////////////////////////////////////////////////
// Scene class functions
// Author: Adam Streck
// Year 2009 - 2011
// 
// Functions that are responsible for: 
//  - building all the directshow objects and setup of windowless rendering
//  - clipping rendering and controlling the video.
//  - creating the drawing bitmap and graphics object
//  - blending the two

// TODO: fix control functions
// TODO: Change my functions to bool

#include "header.h"
#include <evcode.h>
#include <stdio.h>
extern GLOBALS* globals;

extern STATES * game_states;

/**
 * Empty constructor. 
 */
SCENE::SCENE() {
	time_passed = stop_time = 0;
	graphs_built = file_rendered = video_clipped = blending_setup = text_setup = scene_created = false;	
	scene_audio = nullptr;
}

/**
 * Constructor creates all the common parts of the scene - video and audio graphs, finds and renders the source video 
 * and sets up all the variables needed for graphical output. 
 */
SCENE::SCENE(const CMarkup * original_script, STATES * state_pointer) {
	// null values
	source_path = L"error path"; 
	graphs_built = file_rendered = video_clipped = blending_setup = text_setup = scene_created = false;
	fail_msg = L"Non-HRESULT function."; hr = E_FAIL;
	time_passed = stop_time = 0;

	drawing_bitmap = nullptr;
	BMP_mix        = nullptr;
	font_family    = nullptr;
	text_outline   = nullptr;
	text_filler    = nullptr;
	dark_filler    = nullptr;

	game_states = state_pointer;

	// Read the file
	script = CMarkup(*original_script);
	bool file_read = script.FindElem(L"SOURCE");
		Log(L"CMarkup.FindElem(L\"SOURCE\")", MY_FUNC, !file_read);
	source_path = script.GetData();

	// Build scene audio object - for playback of comments
	scene_audio = new AUDIO();
	if (scene_audio == nullptr)
		Log(L"Scene_audio object failed.", DONT_TERM * LOG_FAIL * TAB, true);
	else {
		Log(L"scene_audio->Build()", MY_FUNC * LOG_FAIL * DONT_TERM * TAB, !scene_audio->Build());
		volume = globals->GetGameSettings()->GetValue<int>(L"sound_volume");
	}

	// Build the video
	if(file_read){
		graphs_built   = BuildGraphs();
			if(!graphs_built)   {HRLog(fail_msg.c_str(), hr, true); return;}
		file_rendered  = RenderFile();
			if(!file_rendered)  {HRLog(fail_msg.c_str(), hr, true); return;}
		video_clipped  = ClipVideo();
			if(!video_clipped)  {HRLog(fail_msg.c_str(), hr, true); return;}
		blending_setup = SetupTextBlending();
			if(!blending_setup) {FunctionLog(fail_msg.c_str(), true, true); return;}
		text_setup     = SetupTextProperties();
			if(!text_setup)     {HRLog(fail_msg.c_str(), hr, true); return;}
		scene_created = graphs_built | file_rendered | video_clipped | blending_setup | text_setup;
	}
}

/** 
 * Destructor (safe)
 */
SCENE::~SCENE(){
	if(text_setup){
		delete font_family;
		delete text_outline;
		delete text_filler;
		delete dark_filler;
	}
	if(blending_setup){
		DeleteDC(bitmap_hdc);
		delete drawing_bitmap;
		delete BMP_mix;
	}
	if(graphs_built){
		video_media_control->Stop();
		video_media_event->SetNotifyWindow(NULL, 0, 0); // Redirecting media events back
		bitmap_mixer->Release();
		video_mixer->Release(); 
		windowless_control->Release();
		video_media_control->Release();
		video_media_seeking->Release();
		video_media_event->Release();
		video_volume_control->Release();
		capture_graph->Release();
		video_graph->Release();
	}
	if (scene_audio != nullptr)
		delete scene_audio;
}

/** 
 * Function builds up the video graph and renders it's video. There is no need to release the interfaces - 
 * they will be released through the destructor anyway. 
 */
bool SCENE::BuildGraphs() {
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&capture_graph);
		if(hr != S_OK) {fail_msg = L"CoCreateInstance - ICaptureGraphBuilder2"; return false;}
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&video_graph);
		if(hr != S_OK) {fail_msg = L"CoCreateInstance - IGraphBuilder (video)"; capture_graph->Release(); return false;}
	hr = capture_graph->SetFiltergraph(video_graph);
		if(hr != S_OK) {fail_msg = L"ICaptureGraphBuilder2.SetFiltergraph (IGraphBuilder)"; video_graph->Release(); capture_graph->Release(); return false;}

	hr = video_graph->QueryInterface(IID_IMediaControl, (void **)&video_media_control);
		if(hr != S_OK) {fail_msg = L"QueryInterface - IMediaControl (video)"; video_graph->Release(); capture_graph->Release(); return false;}
	hr = video_graph->QueryInterface(IID_IMediaSeeking, (void **)&video_media_seeking);
		if(hr != S_OK) {fail_msg = L"QueryInterface - IMediaSeeking (video)"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); return false;}
	hr = video_graph->QueryInterface(IID_IMediaEvent, (void **)&video_media_event);
		if(hr != S_OK) {fail_msg = L"QueryInterface - IMediaEvent (video)"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); return false;}
	hr = video_graph->QueryInterface(IID_IBasicAudio,(void **)&video_volume_control);
		if(hr != S_OK) {fail_msg = L"QueryInterface - IBasicAudio (video)"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); video_media_event->Release(); return false;}

	// Registeres a window that will recieve event notifications
	video_media_event->SetNotifyWindow((OAHWND)globals->GetWindow(), WM_GRAPHNOTIFY, 0);
		if(hr != S_OK) {fail_msg = L"IMediaEvent.SetNotifyWindow"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); video_media_event->Release(); return false;}

	// Initializatio of the Video mixer render
	if(!InitWindowlessVMR()) 
		{video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); video_media_event->Release(); return false;}

	// Including the bitmap mixer
	hr = windowless_control->QueryInterface(IID_IVMRMixerBitmap9, (LPVOID *)&bitmap_mixer);
		if(hr != S_OK) {fail_msg = L"QueryInterface - IVMRMixerBitmap9"; video_graph->Release(); capture_graph->Release(); video_media_control->Release(); video_media_seeking->Release(); video_media_event->Release(); return false;}

    return true; 
}

/**
 * Initialization of the Video mixer renderer in the windowless form. The video is embedded into the main window.
 */
bool SCENE::InitWindowlessVMR() { 
	// Video Mixer initialization
    hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&video_mixer); 
		if(hr != S_OK) {fail_msg = L"CoCreateInstance - IBaseFilter (video mixer)"; return false;}
    hr = video_graph->AddFilter(video_mixer, L"Video Mixing Renderer 9");
		if(hr != S_OK) {fail_msg = L"IGraphBuilder.AddFilter (IBaseFilter)"; return false;}

    // Sets the rendering mode and number of streams (used for image blending)
	IVMRFilterConfig9* filter_config; 
    hr = video_mixer->QueryInterface(IID_IVMRFilterConfig9, (void**)&filter_config); 
		if(hr != S_OK) {fail_msg = L"QueryInterface - IVMRFilterConfig9"; video_mixer->Release(); return false;}	
    hr = filter_config->SetRenderingMode(VMR9Mode_Windowless); 
		filter_config->Release(); 
		if(hr != S_OK) {fail_msg = L"IVMRFilterConfig9.SetRenderingMode(VMR9Mode_Windowless)"; video_mixer->Release(); return false;}	

	// Sets the target window to display the video within
    hr = video_mixer->QueryInterface(IID_IVMRWindowlessControl9, (void**)&windowless_control); 
		if(hr != S_OK) {fail_msg = L"QueryInterface - IVMRWindowlessControl9"; video_mixer->Release(); return false;}	
	hr = windowless_control->SetVideoClippingWindow(globals->GetWindow());  
		if(hr != S_OK) {fail_msg = L"QueryInterface - IVMRWindowlessControl9.SetVideoClippingWindow"; windowless_control->Release(); return false;}	

	return true; 
} 

/**
 * Renders the video from the file
 */
bool SCENE::RenderFile() {
	IBaseFilter * Source;

	// Create source filter
	hr = video_graph->AddSourceFilter(source_path.c_str(), L"Source1", &Source);
		if(hr != S_OK) {fail_msg = source_path + std::wstring(L"File loading - function IGraphBuilder.AddSourceFilter"); return false;}
		
	// Render the video and audio and set volume
	hr = capture_graph->RenderStream(0, 0, Source, 0, video_mixer); 
		if(hr != S_OK) {fail_msg = L"ICaptureGraphBuilder2.RenderStream - video"; Source->Release(); return false;}
	hr = capture_graph->RenderStream(0, &MEDIATYPE_Audio, Source, 0, NULL);
		if(hr != S_OK) {fail_msg = L"ICaptureGraphBuilder2.RenderStream - audio"; Source->Release(); return false;}

	Source->Release();

	return true;
}

/**
 * Clips the playback from video dimensions to window dimensions, video is resized to full window width. 
 * It is imporatant to remember, that even though video doesn't fill the space of the window, the pointer can still go 
 * through all the directions, and, more importantly, the object positions are unchanged. 
 */
bool SCENE::ClipVideo(){

	// Find out wheather the performance will be lowered
	hr = windowless_control->GetMaxIdealVideoSize(&video_width, &video_height);
		Log(L"IVMRWindowlessControl9.GetMaxIdealVideoSize()", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);
		if(video_width < window_width) Log(L"Clipping will cause performance lost");

	// Obtain the source video dimensions and compute window video dimensions
	long source_width, source_height;
	hr = windowless_control->GetNativeVideoSize(&source_width, &source_height, NULL, NULL);
		Log(L"IVMRWindowlessControl9.GetNativeVideoSize()", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);
		if(hr != S_OK) {video_width = window_width; video_height = window_height;}
	hr = windowless_control->SetAspectRatioMode(VMR_ARMODE_NONE);
		Log(L"IVMRWindowlessControl9.SetAspectRatioMode()", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);
	
	// Rescale the video so it fits the window. 
	double window_aspect = static_cast<double>(window_height) / static_cast<double>(window_width);
	double video_aspect  = static_cast<double>(source_height) / static_cast<double>(source_width);
	int y_position, x_position;
	if(window_aspect > video_aspect){ // will have top and bottom black border
		video_scale   = static_cast<double>(window_width) / static_cast<double>(source_width);
		video_width  = window_width;
		video_height = static_cast<long>(static_cast<double>(source_height) * video_scale);
		x_position = 0;
		y_position = (window_height - video_height)/2;
	}
	else { // will have left and right black border
		video_scale   = static_cast<double>(window_height) / static_cast<double>(source_height);
		video_height = window_height;
		video_width  = static_cast<long>(static_cast<double>(source_width) * video_scale);
		x_position = (window_width - video_width)/2;
		y_position = 0;
	}

	// clipping
	RECT source_rect; 
	SetRect(&source_rect, 0, 0, source_width, source_height);
	SetRect(&video_position, x_position, y_position, video_width + x_position, video_height + y_position);
	hr = windowless_control->SetVideoPosition(&source_rect, &video_position); // Set the video position.
		if(hr != S_OK) {fail_msg = L"IVMRWindowlessControl9.SetVideoPosition()";return false;}

	return true;
}

/**
 * Setups variables needed for text blending.
 */
bool SCENE::SetupTextBlending() {
	// Builds drawing device
	drawing_bitmap = new Gdiplus::Bitmap(video_width, video_height);
		if (!drawing_bitmap) { fail_msg = L"Bitmap allocation failed"; return false; }
	BMP_mix = new Gdiplus::Graphics(drawing_bitmap);
		if (!BMP_mix) { fail_msg = L"Graphics allocation failed"; delete drawing_bitmap; return false; }

	// Sets the DC
    HDC hdc = GetDC(globals->GetWindow());
		if (hdc == NULL) {fail_msg = L"GetDC()"; delete drawing_bitmap; delete BMP_mix; return false;}
    bitmap_hdc = CreateCompatibleDC(hdc);
    ReleaseDC(globals->GetWindow(), hdc);
		if (bitmap_hdc == NULL) {fail_msg = L"CreateCompatibleDC()"; delete drawing_bitmap; delete BMP_mix; return false;}
 
	// Sets bitmap info 
    ZeroMemory(&bmpInfo, sizeof(bmpInfo));
    bmpInfo.dwFlags = VMR9AlphaBitmap_hDC | VMR9AlphaBitmap_SrcColorKey; // From HDC with blending
    bmpInfo.hdc = bitmap_hdc;
	SetRect(&bmpInfo.rSrc, 0, 0, drawing_bitmap->GetWidth(), drawing_bitmap->GetHeight()); // Whole picture to whole area
    bmpInfo.rDest.top = 0.0f;
	bmpInfo.rDest.left = 0.0f;
    bmpInfo.rDest.right = 1.0f;
    bmpInfo.rDest.bottom = 1.0f;
    bmpInfo.fAlpha = 1.0f; // transparency
	bmpInfo.clrSrcKey = key_color.ToCOLORREF(); // Key color

    return true;
}

/**
 * Mixes the bitmap with the current video.
 */
bool SCENE::BlendText() {
	// connects new bitmap with the DC
    HBITMAP hbitmap;
	drawing_bitmap->GetHBITMAP(key_color, &hbitmap);  // Obtains hbitmap that holds current state of the drawing bitmap 
    HBITMAP hbmOld = (HBITMAP)SelectObject(bitmap_hdc, hbitmap);
		if (hbmOld == 0) {fail_msg = L"SelectObject()"; delete drawing_bitmap; return false;}
 
	// mixes
	bitmap_mixer->SetAlphaBitmap(&bmpInfo);
		Log(L"IVMRMixerBitmap9.SetAlphaBitmap()", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);

	// releases the bitmap
    DeleteObject(SelectObject(bitmap_hdc, hbmOld)); // deletes the HBITMAP
    return true;
}

/**
 * Setup function for variables associated with the text.
 */
bool SCENE::SetupTextProperties(){
	font_family = 0; 
	text_outline = 0; 
	text_filler = 0;
	font_family = new Gdiplus::FontFamily(my_font_family.c_str()); // TODO: change
		if (font_family == 0)  { Log(L"font_family allocation failed.", TERMINATE * LOG_FAIL, 1); return false; }
	text_outline = new Gdiplus::Pen(text_surrounding_color, text_stroke_width); 
		if (text_outline == 0) { Log(L"fext_outline allocation failed.", TERMINATE * LOG_FAIL, 1); delete font_family; return false; }
	text_filler = new Gdiplus::SolidBrush(text_fill_color);
		if (text_filler == 0)  { Log(L"fext_filler allocation failed.", TERMINATE * LOG_FAIL, 1); delete font_family; delete text_outline; return false; }
	dark_filler = new Gdiplus::SolidBrush(nonactive_fill_color);
		if (dark_filler == 0)  { Log(L"dark_filler allocation failed.", TERMINATE * LOG_FAIL, 1); delete text_filler; delete font_family; delete text_outline; return false; }

	Log(L"BMP_mix.SetSmoothingMode()", DONT_TERM * LOG_FAIL * GDI_FUNC, BMP_mix->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias));
	return true;
}

/**
 * After-scene system message posting.
 */
void SCENE::PostSceneFinished(){
	WPARAM wParam = 0;
	LPARAM lParam = 0;
	Log(L"PostMessage SCENE_FINISHED", API_FUNC * TERMINATE, !PostMessage(globals->GetWindow(), SCENE_FINISHED, wParam, lParam));
}

/**
 * After-built system message posting.
 */
void SCENE::PostSceneCreated(){
	WPARAM wParam = 0;
	LPARAM lParam = 0;
	Log(L"PostMessage SCENE_CREATED", API_FUNC, !PostMessage(globals->GetWindow(), SCENE_CREATED, wParam, lParam));	
}

/**
 * Function goes through all the tests and determines wheather they passed or not.
 * It has to be copied int the class scene, because controller and scene goes through different positions in the file
 */
bool SCENE::CheckTests(){
	std::wstring operation, state_name;
	int          value;

	bool fits = false;
	while(script.FindElem(L"TEST")){ // goes through all the tests
		operation = script.GetAttrib(L"op"); // Loads operator
		value = boost::lexical_cast<int, std::wstring>(script.GetAttrib(L"val")); // Makes value into number
		state_name = script.GetData();
		double temp = game_states->state_map.find(state_name)->second; // Finds variable in state system
		
		// wcscmp returns zero if tested strings are equal
		if     (!operation.compare(L">="))
			fits = (temp >= value) ? true : false;
		else if(!operation.compare(L"<="))
			fits = (temp <= value) ? true : false;
		else if(!operation.compare(L"=="))
			fits = (temp == value) ? true : false;
		else if(!operation.compare(L"!="))
			fits = (temp != value) ? true : false;
		else if(!operation.compare(L">"))
			fits = (temp >  value) ? true : false;
		else if(!operation.compare(L"<"))
			fits = (temp <  value) ? true : false;
		else { // Fail in the text file
			std::wstring output(L"Unknown operator associated with state ");
			output.append(state_name).append(L". ");
			Log(output.c_str(), STARS * TERMINATE, 1);
		} 

		if(fits == false)
			return false; // No need for other tests to go on
	}
	return true;
}

/** 
 * Output of video event.
 */
bool SCENE::EventResponse(){
	long event_code, param1, param2;
	hr = video_media_event->GetEvent(&event_code, &param1, &param2, 0);
		HRLog(L"IMediaEvent.GetEvent", hr, false);
	if(SUCCEEDED(hr)){
		std::wstring msg = L"Video event occured. Event code: ";
		msg.append(boost::lexical_cast<std::wstring, int>(event_code));
		msg.append(L".");
		Log(msg, LOG_ALL * TAB * TAB, event_code);
	}
	return true;
}

/**
 * Forced re-paint of the video.
 */
bool SCENE::Repaint(){
	Log(L"Scene repain - Clear(black)", GDI_FUNC * TAB, globals->GetScreenObject()->Clear(Gdiplus::Color(0,0,0)));
	hr = windowless_control->RepaintVideo(globals->GetWindow(), GetWindowDC(globals->GetWindow())); // Draws video on screen ... just to be sure
		Log(L"IVMRWindowlessControl9.RepaintVideo", DX_FUNC, hr);
	return true;
}

/**
 * Playback start.
 */
bool SCENE::Play() {
	video_media_control->Run();
	Log(L"playing audio with scene", TAB * MY_FUNC, !scene_audio->Play());

	// Wait until the transition is complete or 250 ms
	int loop_count = 0;
	do {
		hr = video_media_control->GetState(50, &filter_state);
		Sleep(50);
		loop_count++;
	} while (hr == VFW_S_STATE_INTERMEDIATE && loop_count < 5);
	Log(L"video_media_control->GetState - Video play", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);

	if (filter_state != State_Running) {
		Log(L"Video is not running.", LOG_ALL * TAB * TERMINATE, filter_state);
		return false;
	}

	Repaint();

	// Create timer for the end of the playback, try twice then end
	Log(L"Video seeker GetPositions(&time_passed,&stop_time) at Play()", DX_FUNC * TAB, video_media_seeking->GetPositions(&time_passed,&stop_time));
	if (!SetTimer(globals->GetWindow(), VIDEO_PLAYBACK, static_cast<UINT>((stop_time - time_passed) / 10000), (TIMERPROC) NULL)) {
		Log(L"SetTimer(globals->GetWindow(), VIDEO_PLAYBACK,...)", TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
		if (!SetTimer(globals->GetWindow(), VIDEO_PLAYBACK, static_cast<UINT>((stop_time - time_passed) / 10000), (TIMERPROC) NULL)) {
			Log(L"SetTimer(globals->GetWindow(), VIDEO_PLAYBACK,...)", TAB * LOG_FAIL * TERMINATE *API_FUNC, 1);	
			return false;
		}
	}

	return true;
}

/**
 * Playback pause
 */
bool SCENE::Pause() {
	video_media_control->Pause();
	Log(L"pausing audio with scene", TAB * MY_FUNC, !scene_audio->Pause());

	// Wait until the transition is complete or 250 ms
	int loop_count = 0;
	do {
		hr = video_media_control->GetState(50, &filter_state);
		Sleep(50);
		loop_count++;
	} while (hr == VFW_S_STATE_INTERMEDIATE && loop_count < 5);
	Log(L"video_media_control->GetState - Video pause", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);

	if (filter_state != State_Paused) {
		Log(L"Video was not paused.", LOG_ALL * TAB * TERMINATE, filter_state);
		return false;
	}

	// Kill timer for the end of the playback, try twice then end
	if (!KillTimer(globals->GetWindow(), VIDEO_PLAYBACK)) {
		Log(L"KillTimer(globals->GetWindow(), VIDEO_PLAYBACK)", TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
		if (!KillTimer(globals->GetWindow(), VIDEO_PLAYBACK))
			Log(L"KillTimer(globals->GetWindow(), VIDEO_PLAYBACK)", TAB * LOG_FAIL * TERMINATE *API_FUNC, 1);
	}

	return true;
}

/**
 * Playback stop
 */
bool SCENE::Stop() {
	video_media_control->Stop();
	Log(L"stopping audio with scene", TAB * MY_FUNC, !scene_audio->Stop());

	// Wait until the transition is complete or 250 ms
	int loop_count = 0;
	do {
		hr = video_media_control->GetState(50, &filter_state);
		Sleep(50);
		loop_count++;
	} while (hr == VFW_S_STATE_INTERMEDIATE && loop_count < 5);

	Log(L"video_media_control->GetState - Video stop", DX_FUNC * TAB * DONT_TERM * LOG_FAIL, hr);
	if (filter_state != State_Stopped) {
		Log(L"Video was not stopped.", LOG_ALL * TAB * TERMINATE, filter_state);
		return false;
	}

	// Kill timer for the end of the playback, try twice but do not end
	if (!KillTimer(globals->GetWindow(), VIDEO_PLAYBACK)) {
		Log(L"KillTimer(globals->GetWindow(), VIDEO_PLAYBACK)", TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
		if (!KillTimer(globals->GetWindow(), VIDEO_PLAYBACK))
			Log(L"KillTimer(globals->GetWindow(), VIDEO_PLAYBACK)", TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
	}

	return true;
}

/**
 * Rewinds the stream to the begining
 * Care - does only employ SCENE::PLAY
 */
bool SCENE::Rewind() {
	Log(L"Rewind video Stop()", MY_FUNC, !Stop());

	// Set new positions - start at 0 and don't change end
	LONGLONG null = 0;
	hr = video_media_seeking->SetPositions(&null, AM_SEEKING_AbsolutePositioning, &null, AM_SEEKING_NoPositioning);
	Log(L"video_media_seeking->SetPositions(&null, AM_SEEKING_AbsolutePositioning, &null, AM_SEEKING_NoPositioning);", DX_FUNC * TAB, hr); 

	Log(L"Rewind video Play()", MY_FUNC, !Play());

	return !hr;
}

/**
 * Sets volume
 * put_Volume goes from -10000 to 0
 * -10000 is -100 dB = 0.000 000 000 1	power unit
 * -5000  is -50  dB = 0.000 01 power unit
 * 0      is 0    dB = 1 power unit
 * System volume is in percents - from 0 to 100
 * 0 -> -10000
 * 50 -> -5000
 * 100 ->    0
 */
bool SCENE::SetVolume(int percent) {
	volume = (percent - 100)*100;
	hr = video_volume_control->put_Volume(volume); 
		Log(L"IBasicAudio.put_Volume (video)", DX_FUNC, hr);

	return true;
}

/**
 * Divide the string into parts that take only allowed with (or the smallest width possible) at the position of the cursor
 * and fill the text_path with them.
 */ 
bool SCENE::FillCenteredTextPath(Gdiplus::GraphicsPath & text_path, std::wstring text, Gdiplus::REAL x, Gdiplus::REAL y, float max_width) {
	std::vector<std::wstring> substrings;
	std::vector<int> widths;
	int top_width = 0;
	
	Log(L"Centered text fill - DetermineTextProperties", MY_FUNC * TAB, !DetermineTextProperties(text, max_width, top_width, substrings, widths));

	// Put the bounds and correct them, if out of video
	x = max(0, x - top_width/2);
	x = min(video_width - top_width, x);
	y = max(0, y - (text_size * substrings.size() * 1.1f) /2);
	y = min(video_height - (text_size * substrings.size() * 1.1f), y);

	// Create the real path - center every string
	for (std::size_t i = 0; i != substrings.size(); i++) {
		Gdiplus::REAL my_x = (top_width - widths[i])/2 + x;
		Gdiplus::REAL my_y = y + text_size * i * 1.1f; // The y pos is under previous i lines with 0.1 of text height of spacing
		Log(L"fill centered path text_path.AddString()", GDI_FUNC * TAB, text_path.AddString(substrings[i].c_str(), -1, font_family, Gdiplus::FontStyleRegular, text_size, Gdiplus::PointF(my_x, my_y), &strformat));
	}

	return true;
}

/**
 * Determine how many parts must the string be divided into so it can be dislplayed in requested width.
 * Create this parts.
 * Determine the widht of the widest string.
 */
bool SCENE::DetermineTextProperties(const std::wstring & text, const float max_width, int & width, std::vector<std::wstring> & substrings, std::vector<int> & widths) {
	Gdiplus::Rect bounds;
	Gdiplus::GraphicsPath text_path;
	
	// Count max possible number of parts
	int parts = 0, max_parts = 0;
	for (std::size_t i = 0; i < text.size(); i++)
		if (text[i] == L' ')
			max_parts++;

	// Try to divide unless requested width is ensured or it is not possible to divide any fruther
	do {
	    width = 0;
		substrings.clear();
		widths.clear();
		DivideString(substrings, ++parts, text);
		bounds.Width = bounds.Height = 0;
		// For every substring count and store its width
		for (auto it = substrings.begin(); it != substrings.end(); it++) {
			Log(L"test bounds - text_path.AddString()", GDI_FUNC * TAB, text_path.AddString(it->c_str(), -1, font_family, Gdiplus::FontStyleRegular, text_size, Gdiplus::PointF(0, 0), &strformat));
			Log(L"test bounds - text_path.GetBounds()", GDI_FUNC * TAB, text_path.GetBounds(&bounds, NULL, text_outline));
			widths.push_back(bounds.Width);
			width = max (width, bounds.Width); 
			Log(L"test bounds - text_path.Reset()", GDI_FUNC * TAB, text_path.Reset());
		}
	} while  (width >= (video_width*max_width) && (parts < max_parts));	

	// Test if it even can be inside the video
	if ((bounds.Width > video_width) || ((text_size * substrings.size()) > video_height)) {
		Log(std::wstring(L"String is out of bounds: ") + text, TAB * LOG_ALL, 1);
		return false;
	}

	return true;
}

/**
 * fills the vector with parts of the text that are simmilar in lenght
 */
bool SCENE::DivideString(std::vector<std::wstring> & substrings, const int number, const std::wstring text ) {
	const std::size_t sub_lenght = max (static_cast<int>(text.size()) / number - 1, 0);

	std::size_t current_position = 0;
	std::size_t current_lenght = sub_lenght;

	// Create smallest subparts splitted by ' ' longer tha sub_lenght
	// Return if end is reached
	for (int i = 0; i < number; i++) {
		if (current_position >= text.size())
			return true;
		while ((text.size() > (current_lenght + current_position)) && (text[current_lenght + current_position] != L' '))
			current_lenght++;

		substrings.push_back(text.substr(current_position, current_lenght));
		current_position += (current_lenght+1);
		current_lenght = sub_lenght;
	}
	return true;
}

/** 
 * Get attribute of requested name at current position in the script
 */
std::wstring SCENE::GetAttribute(const std::wstring & name) {
	std::wstring value = script.GetAttrib(name);
	if (value == L"") 
		Log(std::wstring(L"Reading of attribute ").append(name).append(L" failed."), TAB, 1);
	return value;
}
/** 
 * Get data at current position in the script, name of a current item is also logged
 */
std::wstring SCENE::GetData(const std::wstring & name) {
	std::wstring value = script.GetData();
	if (value == L"") 
		Log(std::wstring(L"Reading of data from ").append(name).append(L" failed."), TAB, 1);
	return value;
}