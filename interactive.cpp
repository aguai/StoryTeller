///////////////////////////////////////////////////////////////////////////////////////////////////
// Video class methods
// Author: Adam Streck
// Year 2009 - 2011
// 
// Methods that construct, store and and manage objects. Object respond to mouse movement and 
// mouse buttons pressing.

#include "header.h"
#include "Evcode.h"
extern GLOBALS* globals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions regarding the SCENE_OBJECT class

/**
 * Constructor.
 */
INTERACTIVE::SCENE_OBJECT::SCENE_OBJECT(const Gdiplus::GraphicsPath* boundaries, const std::wstring object_name, 
										const std::wstring object_caption, const std::wstring object_sound_path, 
									    const std::wstring object_caption_sound_path, const std::vector<state_changes> & object_state_changes, 
										const bool finisher){
	position = nullptr;
	position = new Gdiplus::Region(boundaries); 
	if(position == 0)
		Log(L"Unable to create region for object");
	name = object_name; 
	caption = object_caption; 
	sound_path = object_sound_path; 
	caption_sound_path = object_caption_sound_path;
	caused_changes = object_state_changes; 
	finish_scene = finisher;
}

/**
 * Copy constructor.
 */
INTERACTIVE::SCENE_OBJECT::SCENE_OBJECT(const SCENE_OBJECT& image){
	position = nullptr;
	position = image.position->Clone();
	if(position == 0)
		TermLog(L"Unable to create region for object");
	name = image.name;
	caption = image.caption;
	sound_path = image.sound_path;
	caption_sound_path = image.caption_sound_path;
	caused_changes = image.caused_changes;
	finish_scene = image.finish_scene;
}

/**
 * Destructor.
 */
INTERACTIVE::SCENE_OBJECT::~SCENE_OBJECT(){
	caused_changes.clear(); 
	if(position != nullptr)
		delete position;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions regarding the INTERACTIVE class
/**
 * Constructor.
 */
INTERACTIVE::INTERACTIVE(const CMarkup * original_script, STATES* state_pointer) : SCENE(original_script,  state_pointer)
{	
	last_active_object = -1;
	caption_displayed  = false;
	
	if(!scene_created)
		return;
	if(!CreateObjects())
		return;
	if(!Play())
		return;

	PostSceneCreated();	
}

/**
 * Destructor.
 */
INTERACTIVE::~INTERACTIVE(){
	objects.clear();
}

/**
 * Function creates objects from script - first it take names and than it creates path for a region and afterwards the map of state changes.
 */
bool INTERACTIVE::CreateObjects(){
	std::wstring script_file((globals->GetGameSettings())->GetValue<std::wstring>(L"script_file"));

	// attributes
	std::wstring name;
	std::wstring caption;
	std::wstring sound_path;
	std::wstring caption_sound_path;
	bool finish_scene;

	// object's space
	Gdiplus::GraphicsPath boundaries;
	int verticle_count;
	Gdiplus::Point * verticles;
	int x, y;

	// caused changes
	std::string state_name;
	std::vector<state_changes> changes; 

	// control
	bool verticles_OK = true;

	if(!script.FindElem(L"OBJECTS")){
		Log(std::wstring(L"OBJECTS tag not found in ").append(script_file).append(L" file is probably damaged."), TAB * TERMINATE, 1);
		return false;
	}
	script.IntoElem();

	// Objects creation
	if(!script.FindElem(L"OBJECT")){
		script.OutOfElem();
		Log(std::wstring(L"No OBJECT tag found in the OBJECTS list in  ").append(script_file).append(L" file is probably damaged."), TAB * TERMINATE, 1);
		return false;
	}
	
	do {
		// atributes reading
		name = GetAttribute(L"name");
		caption = GetAttribute(L"caption");
		sound_path = GetAttribute(L"sound_source");
		caption_sound_path = GetAttribute(L"caption_sound"); 
		verticle_count = boost::lexical_cast<int,std::wstring>(GetAttribute(L"v_count"));
		if(verticle_count < 3){
			Log(std::wstring(L"Verticle count in object ").append(name).append(L" is too low."), TAB, 1);
			verticles_OK = false;
		} 
		finish_scene =boost::lexical_cast<int,std::wstring>(GetAttribute(L"finish_scene")) != 0 ? true : false;
		
		script.IntoElem(); // Jumps into the object

		// Tests that resolve wheather the object is used at all
		if(!CheckTests()){
			script.OutOfElem();
			continue;
		}

		// creates verticles for the object polygon
		verticles = new Gdiplus::Point [verticle_count];
		for(int i = 0; i < verticle_count; i++){
			if(!script.FindElem(L"VERTEX")){
				Log(L"There is not responding count of verticles in one object in script.");
				verticles_OK = false;
			}
			x = boost::lexical_cast<int,std::wstring>(GetAttribute(L"x")); 
			y = boost::lexical_cast<int,std::wstring>(GetAttribute(L"y")); 
			verticles[i] = Gdiplus::Point(x, y);
		}
		if(!verticles_OK) {
			delete [] verticles;
			script.OutOfElem();
			continue; // skips the whole creation of this object
		}
		else {
			boundaries.AddPolygon(verticles, verticle_count); // Create polygonal path
			delete [] verticles;
		}

		// creates the vector of state changes
		while(script.FindElem(L"STATECHANGE")){
			state_changes current_changes;
			current_changes.name = GetData(L"STATECHANGE");
			current_changes.value = boost::lexical_cast<int, std::wstring>(GetAttribute(L"value"));
			current_changes.replace = boost::lexical_cast<bool, std::wstring>(GetAttribute(L"replace"));
			changes.push_back(current_changes);
		}
		
		// Insert object and free used data
		objects.push_back(SCENE_OBJECT(&boundaries, name, caption, sound_path, caption_sound_path, changes, finish_scene));
		changes.clear();
		boundaries.Reset();
		script.OutOfElem(); // Jumps out of the object

	} while(script.FindElem(L"OBJECT"));

	script.OutOfElem();
	return true;
}

/**
 * Function displays strings of the text, when needed.
 */
void INTERACTIVE::DrawString(const Gdiplus::PointF & hit_position, const std::wstring & text){

	FillCenteredTextPath(path, text, static_cast<Gdiplus::REAL>(hit_position.X), static_cast<Gdiplus::REAL>(hit_position.Y), 0.5f);

	// Draw on the screen
	Log(L"Graphic.DrawPath()", API_FUNC * TAB, BMP_mix->DrawPath(text_outline, &path));	
	Log(L"Graphic.FillPath()", API_FUNC * TAB, BMP_mix->FillPath(text_filler, &path));
	
	// remove data from path
	Log(L"GraphicPath.Reset()", API_FUNC * TAB, path.Reset()); 
}

/**
 * Rewinds the video after it reached the end
 */
bool INTERACTIVE::VideoEnded(){
	return Rewind();
}

/**
 * Left click - acivates the object clicked on.
 */
bool INTERACTIVE::LeftClick(const int x, const int y){
	// Change the coordinates if the video is not in the full window
	Gdiplus::PointF	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);

	if(i != -1){
		std::wstring source = objects[i].sound_path;
		// Play audio, if it the object has any
		if (source.compare(L"!sound_path!") != 0) {
			scene_audio->RenderNewFile(objects[i].sound_path, volume);
		}	

		// Apply changes & decide wheather the scene shoudl finish
		if(!objects[i].caused_changes.empty())
			game_states->ApplyChanges(objects[i].caused_changes);
		if(objects[i].finish_scene)
			PostSceneFinished();
	}
	return true;
}

/**
 * Right click - get info about the object.
 */
bool INTERACTIVE::RightClick(const int x, const int y){
	// Change the coordinates if the video is not in the full window
	Gdiplus::PointF	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);

	if(i != -1){
		// Play audio, if it the object has any
		std::wstring source = objects[i].caption_sound_path;
		if (source.compare(L"!caption_sound_path!") != 0) 
			scene_audio->RenderNewFile(objects[i].caption_sound_path, volume);

		// Display the caption
		caption_displayed = true;
		BMP_mix->Clear(key_color);
		DrawString(hit_position, objects[i].caption.c_str());
	}	
	BlendText();
	return true;
}

/**
 * Respond to the move of the cursor - if there is a object hit, object is captioned.
 */
bool INTERACTIVE::MouseMove(const int x,const int y){
	Gdiplus::PointF	hit_position(x - static_cast<Gdiplus::REAL>(video_position.left), y - static_cast<Gdiplus::REAL>(video_position.top));  		
	int i = DetectCollision(hit_position);

	// If new object was hit
	if(i != -1 && i != last_active_object){
		last_active_object = i;
		caption_displayed = false;

		BMP_mix->Clear(key_color);
		DrawString(hit_position, objects[i].name.c_str());
		BlendText();
	}	
	else if (last_active_object != -1 && i == -1 && caption_displayed != true){	
		BMP_mix->Clear(key_color);
		last_active_object = -1;			
		BlendText();
	}
	return true;
}

/**
 * Finds out wheather there is a collision with any object.
 */
int INTERACTIVE::DetectCollision(const Gdiplus::PointF & hit_position){
	unsigned int i = 0; // i stores number of the intersecting object, if i is equal to number of objects, hit wasn't detected -> returns 0
	for(std::vector<SCENE_OBJECT>::iterator it = objects.begin(); it < objects.end(); it++)
		if (it->position->IsVisible(hit_position))
			break;
		else 
			i++;
	return (i < (objects.size()) ) ? i : -1;	
}

