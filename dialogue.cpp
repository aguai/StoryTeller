///////////////////////////////////////////////////////////////////////////////////////////////////
// Dialogue class functions
// Author: Adam Streck
// Year 2009 - 2011
// 
// Functions are responsible for creation of dialogue options, thier management and reactions to
// user input.

#include "header.h"
extern GLOBALS* globals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions regarding the OPTION class
/**
 * Constructor.
 */
DIALOGUE::OPTION::OPTION(std::wstring & object_text, Gdiplus::GraphicsPath * object_path, std::vector<state_changes> & object_state_changes){
	text = object_text;
	text_path = nullptr;
	text_path = object_path->Clone();
	boundaries = nullptr;
	boundaries = new Gdiplus::Rect;
	text_path->GetBounds(boundaries);
	caused_changes = object_state_changes;
}

/**
 * Copy constructor.
 */
DIALOGUE::OPTION::OPTION(const OPTION& original){
	text = original.text;
	text_path = nullptr;
	text_path = original.text_path->Clone();
	boundaries = nullptr;
	boundaries = new Gdiplus::Rect;
	text_path->GetBounds(boundaries);
	caused_changes = original.caused_changes;
}

/**
 * Destructor.
 */
DIALOGUE::OPTION::~OPTION(){
	caused_changes.clear();
	if(text_path != nullptr)
		delete text_path;
	if(boundaries != nullptr)
		delete boundaries;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions regarding the DIALOGUE class

/**
 * Constructor.
 */
DIALOGUE::DIALOGUE(const CMarkup * original_script,  STATES* state_pointer) : SCENE(original_script, state_pointer) {

	// Create positioning
	Gdiplus::Font temp_font(font_family, text_size, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	line_height = static_cast<int>(temp_font.GetHeight(BMP_mix)) ; // sets the difference between two lines, in pixels
	line_bot    = static_cast<int>(video_height*0.95f);             // sets top of the zero line 25 pixels beyond the bottom of the window
	left_space  = static_cast<int>(video_width *0.015f);             // sets number of pixels from left border

	highlited_option = 0; // TODO: Change to -1

	if(!scene_created)
		return;
	if(!CreateOptions())
		return;
	if(!DrawOptions())
		return;
	if(!Play())
		return;

	PostSceneCreated();
}

/**
 * Destructor.
 */
DIALOGUE::~DIALOGUE(){
	options.clear();
}

/**
 * Creating of the options from the script.
 */
bool DIALOGUE::CreateOptions(){
	std::wstring script_file((globals->GetGameSettings())->GetValue<std::wstring>(L"script_file"));

	// atributes
	std::wstring text;
	Gdiplus::GraphicsPath path;
	// caused changes which will be paired with the options
	std::string state_name;
	std::vector<state_changes> changes; 

	if(!script.FindElem(L"OPTIONS")) {
		Log(std::wstring(L"OPTIONS tag not found in ").append(script_file).append(L" file is probably damaged."), TAB * TERMINATE, 1);
		return false;
	}
	script.IntoElem();

	// Going through the options
	if(!script.FindElem(L"OPTION")){
		script.OutOfElem();
		Log(std::wstring(L"No OPTION tag found in the OPTIONS list in  ").append(script_file).append(L" file is probably damaged."), TAB * TERMINATE, 1);
		return false;
	}

	do {
		text = GetAttribute(L"text");
		script.IntoElem();

		// Tests that resolve wheather the object is used at all
		if(!CheckTests()){
			script.OutOfElem();
			continue;
		}

		// Create the path
		CreateOptionsGraphics(text, path);

		// creates the map of state changes
		while(script.FindElem(L"STATECHANGE")){
			state_changes current_changes;
			current_changes.name = GetData(L"STATECHANGE");
			current_changes.value = boost::lexical_cast<int, std::wstring>(GetAttribute(L"value"));
			current_changes.replace = boost::lexical_cast<int, std::wstring>(GetAttribute(L"replace"))  != 0 ? true : false;
			changes.push_back(current_changes);
		}
		
		// Store the option
		options.push_back(OPTION(text, &path, changes));

		// Free used obejcts
		script.OutOfElem();
		changes.clear();
		Log(L"GraphicPath.Reset()", GDI_FUNC * TAB, path.Reset());

	} while (script.FindElem(L"OPTION"));

	script.OutOfElem();
	return true;
}

/**
 * Creates the 
 */
bool DIALOGUE::CreateOptionsGraphics(const std::wstring & text, Gdiplus::GraphicsPath & text_path) {
	std::vector<std::wstring> substrings;
	std::vector<int> widths;
	int top_width = 0;

	DetermineTextProperties(text, 0.85f, top_width, substrings, widths);
	for (std::size_t i = 0; i < substrings.size(); i++) {
		Log(L"fill centered path text_path.AddString()", GDI_FUNC * TAB,  text_path.AddString(text.c_str(), -1, font_family, Gdiplus::FontStyleBold, text_size, Gdiplus::PointF(Gdiplus::REAL(left_space), Gdiplus::REAL(line_bot)), &strformat));
		line_bot -= line_height*1.1f;
	}
	line_bot -= line_height*0.1f;

	return true;
}

/**
 * Drawing of the options on the screen.
 */
bool DIALOGUE::DrawOptions(){
	StatusLog(L"Graphic.Clear()", BMP_mix->Clear(key_color), false);

	//  Display all the options - if one of them is highlighted, use different filler
	int i = 1;
	for(std::vector<OPTION>::iterator it = options.begin(); it < options.end(); it++, i++) {
		Log(L"Graphic.DrawPath()", GDI_FUNC * TAB, BMP_mix->DrawPath(text_outline, it->text_path));	
		if(i != highlited_option)
			Log(L"Graphic.FillPath()", GDI_FUNC * TAB, BMP_mix->FillPath(dark_filler, it->text_path));
		else
			Log(L"Graphic.FillPath()", GDI_FUNC * TAB, BMP_mix->FillPath(text_filler, it->text_path));
	}
	return BlendText();		
}

/**
 * Detection of the collision with the text.
 */
int DIALOGUE::DetectCollision(const Gdiplus::Point & hit_position){
	unsigned int i = 1; // i stores number of the intersecting object, if i is equal to number of objects, hit wasn't detected -> returns 0
	
	for(std::vector<OPTION>::iterator it = options.begin(); it < options.end(); it++)
		if (it->boundaries->Contains(hit_position))
			break;
		else 
			i++;

	return (i != (options.size() + 1) ) ? i : 0; // if i is equal to count of options + 1, it means there were no hit -> returns 0 
}

/**
 * Left click - activation of the option.
 */
bool DIALOGUE::LeftClick(const int x, const int y){
	Gdiplus::Point	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);
	
	// If mouse is currently over any item
	if(i){
		if(!options[i-1].caused_changes.empty())
			game_states->ApplyChanges(options[i-1].caused_changes);
		PostSceneFinished(); 
	}

	return true;
}

/**
 * Right click - no assignment.
 */
bool DIALOGUE::RightClick(const int x, const int y){
	return S_OK;
}

/**
 * Mouse move - detection of the collision with the option.
 */
bool DIALOGUE::MouseMove(const int x, const int y){
	Gdiplus::Point hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);

	if(i != highlited_option){ // Mouse moved to the new option or out
		highlited_option = i;
		DrawOptions();
	}	
	return true;
}

/* Update - end of the video check. */
bool DIALOGUE::VideoEnded(){
	Rewind();
	return true;
}
