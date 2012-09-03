///////////////////////////////////////////////////////////////////////////////////////////////////
// Scene class
// Author: Adam Streck
// Year 2009 - 2011
// 
// Basic class that is build every time a game scene changes. Scene mainly covers building,
// rendering and controlling the video - which is part of each scene type. It also has objects
// and functions necessary for drawing and mixing (blending) the picture with the video.
// Few other objects and functions is employed, so they can be used in derived clases.

// TODO make abstract

#ifndef DEF_ENGINE_SCENE
#define DEF_ENGINE_SCENE

#include "audio.h"
#include <D3D9.h>
#include <Vmr9.h>
#include <gdiplus.h>
#include "Markup.h"

class STATES;
class AUDIO;

class SCENE {
private:

protected:
	// Copies of controller properties
	STATES * game_states;
	CMarkup script;	

	// Video stream path - the content is read before the file is rendered
	std::wstring source_path;

	// system control
	HRESULT hr;	
	Gdiplus::Status stat;
	std::wstring fail_msg; // Used during building of the Scene
	bool graphs_built, file_rendered, video_clipped, blending_setup, text_setup, scene_created;
	OAFilterState filter_state;

	// Playback values
	RECT     video_position;
	long     video_width;
	long     video_height;
	double   video_scale;
	LONGLONG time_passed;
	LONGLONG stop_time;

	// Audio properties
	AUDIO * scene_audio;
	int volume;

	// Video interfaces
	IGraphBuilder          *video_graph;
    IMediaControl          *video_media_control;
	IMediaSeeking		   *video_media_seeking;
    IMediaEventEx          *video_media_event;
	IBasicAudio            *video_volume_control;
	IBaseFilter            *video_mixer;
	ICaptureGraphBuilder2  *capture_graph;
	IVMRWindowlessControl9 *windowless_control;
	IVMRMixerBitmap9       *bitmap_mixer;

	// Blending objects
	Gdiplus::Graphics* BMP_mix;
	Gdiplus::Bitmap*   drawing_bitmap;
	HDC                bitmap_hdc;
	VMR9AlphaBitmap    bmpInfo;

	// text's objects
	Gdiplus::FontFamily*  font_family;
	Gdiplus::Pen*		  text_outline; 
	Gdiplus::SolidBrush*  text_filler;
	Gdiplus::SolidBrush*  dark_filler;
	Gdiplus::StringFormat strformat;
	Gdiplus::GraphicsPath path;
	Gdiplus::Rect		  bounding_rect;

public:
	SCENE();
	SCENE(const SCENE &) { throw (L"SCENE copy contuctor has been called"); }
	SCENE& operator=(const SCENE &) { throw (L"SCENE equality operator has been called"); }
	SCENE(const CMarkup * original_script, STATES* game_states);
	virtual ~SCENE();

	// Building functions
	bool BuildGraphs();
	bool BuildAudio();
	bool InitWindowlessVMR();
	bool RenderFile();
	bool ClipVideo();
	bool SetupTextBlending();
	bool SetupTextProperties();

	// Playback control
	virtual bool Play();
	virtual bool Pause();
	virtual bool Stop();
	        bool Rewind();
	virtual bool Repaint();

	// Interactivity functions
	virtual bool LeftClick(const int x, const int y) {return true; }
	virtual bool RightClick(const int x, const int y) {return true; }
	virtual bool MouseMove(const int x, const int y) {return true; }
	
	// Internal engine functions
	virtual bool EventResponse();	
	virtual bool VideoEnded( void ) = 0;
	bool SetVolume(int percent); // Sets volume of the video and scene audio
	void PostSceneFinished();
	void PostSceneCreated(); // Obsolete
	bool CheckTests();
	long GetRunTime() { video_media_seeking->GetPositions(&time_passed,&stop_time); return static_cast<long>(time_passed/10000);}

	// Text management functions
	bool BlendText();
	bool FillCenteredTextPath(Gdiplus::GraphicsPath & text_path, std::wstring text, Gdiplus::REAL x, Gdiplus::REAL y, float max_width);
	bool DetermineTextProperties(const std::wstring & text, const float max_width, int & width, std::vector<std::wstring> & substrings, std::vector<int> & widths); 
	bool DivideString(std::vector<std::wstring> & substrings, const int number, const std::wstring text);

	// Script management functions
	std::wstring GetAttribute(const std::wstring & name);
	std::wstring GetData(const std::wstring & name);
};

#endif