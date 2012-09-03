///////////////////////////////////////////////////////////////////////////////////////////////////
// Video class methods
// Author: Adam Streck
// Year 2009 - 2011
// 
// Methods that construct, store and display subtitles

#include "header.h"
extern GLOBALS* globals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions regarding the SUBTITLE class

/**
 * Constructor.
 */
VIDEO::SUBTITLE::SUBTITLE(std::wstring sub_text = L"", LONGLONG sub_start = 0, LONGLONG sub_end = 0){
	text = sub_text;
	time_start = sub_start;
	time_end = sub_end;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions regarding the VIDEO class

/**
 * Constructor.
 */
VIDEO::VIDEO(const CMarkup * original_script, STATES*  state_pointer) : SCENE(original_script,  state_pointer)
{
	if(!scene_created)
		return;
	if(!CreateSubtitles())
		return;
	if(!Play())
		return;

	PostSceneCreated();
}
/**
 * Destructor.
 */
VIDEO::~VIDEO(){
	while (ShowCursor(true) < 0) continue;
	scene_subtitles.clear();
}

/**
 * Loading the subtitles from the script.
 */
bool VIDEO::CreateSubtitles(){
	// atributes
	std::wstring text;
	LONGLONG start;
	LONGLONG end;

	// Find the part with subtitles
	if(!script.FindElem(L"SUBTITLES")){
		Log(L"SUBTITLES tag not found in the VIDEO scene, script.xml file is probably damaged.", TERMINATE * TAB * LOG_ALL, 1);
		return false;
	}
	script.IntoElem();

	// Going through the subtitles
	while(script.FindElem(L"SUBTITLE")){
		text  = GetData(L"SUBTITLE");
		start = boost::lexical_cast<LONGLONG,std::wstring>(GetAttribute(L"start"));
		end = boost::lexical_cast<LONGLONG,std::wstring>(GetAttribute(L"end"));

		scene_subtitles.push_back(SUBTITLE(text, start, end));
	}

	script.OutOfElem();
	return true;
}

/**
 * Creates timers that eventually display and hide subtitles
 * Display is called for the next subtitle
 * Hide is called for the active subtitle, if any. Otherwise it is also called for the next subtitle.
 */
bool VIDEO::ScheduleNextSubtitles(void) {
	hr = video_media_seeking->GetPositions(&time_passed,&stop_time);

	int next_number = GetNextSub();
	int act_number  = GetActiveSub();
	if (next_number == -1)
		return true;
	auto sub = scene_subtitles[next_number];

	std::wstring error_message(L"subtitle ");
	error_message.append(sub.text).append(L" SetTimer(globals->GetWindow(), SUBTITLES, "); 

	if (next_number == 0) // Because loading takes time, first subtitles tend to be scheduled for too soon
		time_passed = 0;
	if (!SetTimer(globals->GetWindow(), SUBTITLES_START, static_cast<UINT>(((sub.time_start) - time_passed)/10000), (TIMERPROC) NULL)) {
		Log(error_message.append(L"_START,...)"), TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
		return false;
	}

	if (act_number != -1)
		sub = scene_subtitles[act_number];
	if (!SetTimer(globals->GetWindow(), SUBTITLES_END  , static_cast<UINT>(((sub.time_end) - time_passed)/10000), (TIMERPROC) NULL)) {
		Log(error_message.append(L"_END  ,...)"), TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
		return false;
	}

	return true;
}

/**
 * Obtains subtitles that should be displayed now.
 */
int VIDEO::GetActiveSub(void) {
	int counter = scene_subtitles.size();
	Log(L"Active subtitles video_media_seeking->GetPositions(&time_passed,&stop_time)", DX_FUNC, video_media_seeking->GetPositions(&time_passed,&stop_time));
	for (auto it = scene_subtitles.rbegin(); it != scene_subtitles.rend(); it++) {
		counter--;
		if (time_passed>= it->time_start && time_passed <= it->time_end)
			return counter;
	}
	return -1;
}

/**
 * Obtains subtitles that should be displayed next.
 */
int VIDEO::GetNextSub(void) {
	int counter = 0;
	Log(L"Next subtitles video_media_seeking->GetPositions(&time_passed,&stop_time)", DX_FUNC, video_media_seeking->GetPositions(&time_passed,&stop_time));
	for (auto it = scene_subtitles.begin(); it != scene_subtitles.end(); it++) {
		if (time_passed <= it->time_start)
			return counter;
		counter++;
	}
	return -1;
}


/**
 * Kills timers when the system is not running.
 */
bool VIDEO::KillTimers(void) {
	bool test_1 = true, test_2 = true;
	if (!KillTimer(globals->GetWindow(), SUBTITLES_START)) {
		Log(L"KillTimer(globals->GetWindow(), SUBTITLES_START)", TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
		test_1 = false;
	}
	if (!KillTimer(globals->GetWindow(), SUBTITLES_END)) {
		Log(L"KillTimer(globals->GetWindow(), SUBTITLES_END)", TAB * LOG_FAIL * DONT_TERM *API_FUNC, 1);
		test_2 = false;
	}
	return (test_1 & test_2);	
}


/**
 * Displaying of the subtitle on the screen.
 */
bool VIDEO::DrawSubtitles(bool display){
	BMP_mix->Clear(key_color);

	if(display) {
		int next_number = GetActiveSub();
		if (next_number == -1)
			return false;

		Gdiplus::GraphicsPath text_path;
		std::wstring text = scene_subtitles[next_number].text;
		FillCenteredTextPath(text_path, text, static_cast<Gdiplus::REAL>(video_width / 2.0), static_cast<Gdiplus::REAL>(video_height * 0.9), 0.7f);

		// Draw on the screen
		Log(L"Graphic.DrawPath()", API_FUNC * TAB, BMP_mix->DrawPath(text_outline, &text_path));	
		Log(L"Graphic.FillPath()", API_FUNC * TAB, BMP_mix->FillPath(text_filler, &text_path));

		Log(L"GraphicPath.Reset()", API_FUNC * TAB, text_path.Reset()); // remove path
	}

	BlendText();
	return true;
}

/**
 * Playback start.
 */
bool VIDEO::Play(){
	while (ShowCursor(false) >= 0) continue;
	SCENE::Play();
	return ScheduleNextSubtitles();
}

/**
 * Playback pause
 */
bool VIDEO::Pause(){
	while (ShowCursor(true) < 0) continue;
	SCENE::Pause();
	return KillTimers();
}

/**
 * Left mouse click - nothing.
 */
bool VIDEO::LeftClick(const int x, const int y){
	return true;
}

/**
 * Right mouse click mouse click - ends the video prematurely.
 */
bool VIDEO::RightClick(const int x, const int y){
	return VideoEnded();
}

/**
 * Mouse move - nothing.
 */
bool VIDEO::MouseMove(const int x, const int y){
	return true;
}

/**
 * Reaction to the end of the playback.
 */
bool VIDEO::VideoEnded( void ) {
	Stop();
	// Timers can be still active - scene ended before SUBTITLES_END message or user clicked the right button - must be killed
	KillTimer(globals->GetWindow(), SUBTITLES_START);
	KillTimer(globals->GetWindow(), SUBTITLES_END);
	PostSceneFinished();
	return true;
}