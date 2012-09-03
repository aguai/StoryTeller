// Dont redo now

#include "header.h"
extern GLOBALS* globals;

// Button paths
const wchar_t * background_path          = L"pictures/menu/background.jpg";
const wchar_t * new_basic_path           = L"pictures/menu/new_b.bmp";
const wchar_t * new_higlited_path        = L"pictures/menu/new_h.bmp";
const wchar_t * new_activated_path       = L"pictures/menu/new_a.bmp";
const wchar_t * exit_basic_path          = L"pictures/menu/exit_b.bmp";
const wchar_t * exit_higlited_path       = L"pictures/menu/exit_h.bmp";
const wchar_t * exit_activated_path      = L"pictures/menu/exit_a.bmp";
const wchar_t * continue_basic_path      = L"pictures/menu/continue_b.bmp";
const wchar_t * continue_higlited_path   = L"pictures/menu/continue_h.bmp";
const wchar_t * continue_activated_path  = L"pictures/menu/continue_a.bmp";
const wchar_t * slider_basic_path        = L"pictures/menu/volume_b.bmp";
const wchar_t * slider_higlited_path     = L"pictures/menu/volume_h.bmp";
const wchar_t * slider_slider_path       = L"pictures/menu/volume_s.bmp";
const wchar_t * fullscreen_basic_path    = L"pictures/menu/fullscreen_b.bmp";
const wchar_t * fullscreen_higlited_path = L"pictures/menu/fullscreen_h.bmp";
const wchar_t * fullscreen_activated_path= L"pictures/menu/fullscreen_s.bmp";

// Functions regarding the BUTTON class------------------------------------------
/* Button constructor. */
GAMEMENU::BUTTON::BUTTON(const int x, const int y, const wchar_t *path_basic, const wchar_t *path_highlited, const wchar_t *path_activated, int button_ID) {
	picture_basic = picture_activated = picture_highlited = 0;
	picture_basic = new Gdiplus::Image(path_basic);
	picture_highlited = new Gdiplus::Image(path_highlited);
	picture_activated = new Gdiplus::Image(path_activated);
		if (picture_basic == 0) TermLog(L"Failed to load button picture, probably out of memory");
		if (picture_highlited == 0 || picture_activated == 0) Log(L"Failed to load button special picture"); // No need to turn out
	position = Gdiplus::Rect(x, y, picture_basic->GetWidth(), picture_basic->GetHeight());
	active_picture = basic;
	ID = button_ID; // is used for actions
}

/* Button copying constructor */
GAMEMENU::BUTTON::BUTTON(const BUTTON & image) {
	picture_basic = picture_activated = picture_highlited = 0;
	position = Gdiplus::Rect(image.position);
	picture_basic = image.picture_basic->Clone();
	picture_highlited = image.picture_highlited->Clone();
	picture_activated = image.picture_activated->Clone();
	active_picture = image.active_picture;
	ID = image.ID;
}

/* Button equation operator */
GAMEMENU::BUTTON& GAMEMENU::BUTTON::operator=(const BUTTON & image) {
	position = Gdiplus::Rect(image.position);
	picture_basic = image.picture_basic->Clone();
	picture_highlited = image.picture_highlited->Clone();
	picture_activated = image.picture_activated->Clone();
	active_picture = image.active_picture;
	ID = image.ID;
	return *this;
}

/* Button destructor */
GAMEMENU::BUTTON::~BUTTON() {
	delete picture_activated;
	delete picture_highlited;
	delete picture_basic;
}

/* Button draw function */
void GAMEMENU::BUTTON::Draw() {
	if(active_picture == basic)
		StatusLog(L"BMP->DrawImage(picture_basic, position.X, position.Y)", "Button drawing failed.", globals->GetCanvasObject()->DrawImage(picture_basic, position.X, position.Y), false);
	if(active_picture == highlited) // mouse is over the button
		StatusLog(L"BMP->DrawImage(picture_highlited, position.X, position.Y)", "Button drawing failed.", globals->GetCanvasObject()->DrawImage(picture_highlited, position.X, position.Y), false);
}

// Functions regarding the SLIDER class------------------------------------------
/* Sldier constructor. */
GAMEMENU::SLIDER::SLIDER(const int x, const int y, const wchar_t *path_basic, const wchar_t *path_highlited, const wchar_t *path_slider, int button_ID, int input_value) {
	picture_basic = picture_slider = picture_highlited = 0;
	picture_basic = new Gdiplus::Image(path_basic);
	picture_highlited = new Gdiplus::Image(path_highlited);
	picture_slider = new Gdiplus::Image(path_slider);
		if (picture_basic == 0) TermLog(L"Failed to load button picture, probably out of memory");
		if (picture_highlited == 0 || picture_slider == 0) Log(L"Failed to load button special picture"); // No need to turn out
	position = Gdiplus::Rect(x, y, picture_basic->GetWidth(), picture_basic->GetHeight());
	active_picture = basic;
	ID = button_ID; // is used for actions
	value = input_value;
	slider_width = picture_slider->GetWidth();
}

/* Button copying constructor */
GAMEMENU::SLIDER::SLIDER(const SLIDER & image) {
	picture_basic = picture_slider = picture_highlited = 0;
	position = Gdiplus::Rect(image.position);
	picture_basic = image.picture_basic->Clone();
	picture_highlited = image.picture_highlited->Clone();
	picture_slider = image.picture_slider->Clone();
	active_picture = image.active_picture;
	ID = image.ID;
	value = image.value;
	slider_width = image.slider_width;
}

/* Button equation operator */
GAMEMENU::SLIDER& GAMEMENU::SLIDER::operator=(const SLIDER & image) {
	picture_basic = picture_slider = picture_highlited = 0;
	position = Gdiplus::Rect(image.position);
	picture_basic = image.picture_basic->Clone();
	picture_highlited = image.picture_highlited->Clone();
	picture_slider = image.picture_slider->Clone();
	active_picture = image.active_picture;
	ID = image.ID;
	value = image.value;
	slider_width = image.slider_width;
	return *this;
}

/* Button destructor */
GAMEMENU::SLIDER::~SLIDER() {
	delete picture_slider;
	delete picture_highlited;
	delete picture_basic;
}

/* Button draw function */
void GAMEMENU::SLIDER::Draw() {
	if(active_picture == basic)
		StatusLog(L"BMP->DrawImage(picture_basic, position.X, position.Y)", "Button drawing failed.", globals->GetCanvasObject()->DrawImage(picture_basic, position.X, position.Y), false);
	if(active_picture == highlited) // mouse is over the button
		StatusLog(L"BMP->DrawImage(picture_highlited, position.X, position.Y)", "Button drawing failed.", globals->GetCanvasObject()->DrawImage(picture_highlited, position.X, position.Y), false);
	float slider_pos = (static_cast<float>(picture_basic->GetWidth() - picture_slider->GetWidth())/100.0f)*static_cast<float>(value);
		StatusLog(L"BMP->DrawImage(picture_slider, position.X, position.Y)", "Button drawing failed.", globals->GetCanvasObject()->DrawImage(picture_slider, position.X + static_cast<int>(slider_pos), position.Y), false);
}

// Functions regarding the MENU class--------------------------------------------
/* Menu creation. */
GAMEMENU::GAMEMENU(){
	int X_position = upper_left_corner_X + (window_width/2) - 150;

	// Buttons loading
	background_picture = new Gdiplus::Image(background_path);
	MENU_ITEM * temp = new BUTTON(X_position, upper_left_corner_Y + 200, new_basic_path, new_higlited_path, new_activated_path, 1);
	menu_items.push_back(temp);
	temp = new BUTTON(X_position, upper_left_corner_Y + 100, continue_basic_path, continue_higlited_path, continue_activated_path, 3);
	menu_items.push_back(temp);
	temp = new BUTTON(X_position, upper_left_corner_Y + 300, fullscreen_basic_path, fullscreen_higlited_path, fullscreen_activated_path, 5);
	menu_items.push_back(temp);
	temp = new SLIDER(X_position, upper_left_corner_Y + 400, slider_basic_path, slider_higlited_path, slider_slider_path, 4, 100);
	menu_items.push_back(temp);
	temp = new BUTTON(X_position, upper_left_corner_Y + 500, exit_basic_path, exit_higlited_path, exit_activated_path, 2);
	menu_items.push_back(temp);

	FunctionLog(L"StartUP Redraw call.", Redraw(), false);
	FunctionLog(L"StartUP Display call.", Display(), false);
}

/* Menu destructor. */
GAMEMENU::~GAMEMENU(){
	for(std::vector<MENU_ITEM*>::iterator it = menu_items.begin(); it < menu_items.end(); it++)
		delete (*it);
	menu_items.clear();
	if(background_picture != 0){
		delete background_picture;
		background_picture = 0;
	}
}

/* MouseMove response - looks up for button collision. */
bool GAMEMENU::MouseMove(const int x, const int y){
	// change pictures of buttons
	for(std::vector<MENU_ITEM*>::iterator it = menu_items.begin(); it < menu_items.end(); it++){
		(*it)->active_picture = MENU_ITEM::basic;
		if ((*it)->position.Contains(x, y))
			(*it)->active_picture = MENU_ITEM::highlited;
	}
	FunctionLog(L"MouseMove Redraw call.", Redraw(), false);
	FunctionLog(L"MouseMove Display call.", Display(), false);
	return true;
}
/* LeftClick response - activates the button if the collision was found. */
bool GAMEMENU::LeftClick(const int x, const int y){
	// Lists through the buttons to find proper function
	for(std::vector<MENU_ITEM*>::iterator it = menu_items.begin(); it < menu_items.end(); it++){
		if ((*it)->position.Contains(x, y)){
			if((*it)->ID == 1) // Start
				GameStart();
			else if((*it)->ID == 2) // Exit
				ExitGame();
			else if((*it)->ID == 3)
				Continue();
			else if((*it)->ID == 4){
				int new_value = ChangeVolume(x-(*it)->position.GetLeft(), (*it)->position.Width);
				dynamic_cast<SLIDER*>(*it)->SetValue(new_value);
				FunctionLog(L"MouseMove Redraw call.", Redraw(), false);
				FunctionLog(L"MouseMove Display call.", Display(), false);
			}
			else if((*it)->ID == 5){
				Fullscreen();
			}
		}
	}
	return true;
}
/* RightClick response - currently none. */
bool GAMEMENU::RightClick(const int x, const int y) {
	return true;
}
/* Redraw was forced - all pictures are redrawn at their state. */
bool GAMEMENU::Redraw() { 
	StatusLog(L"BMP->DrawImage(background_picture, 0, 0, window_width, window_height)", globals->GetCanvasObject()->DrawImage(background_picture, 0, 0, window_width, window_height), false);
	for(std::vector<MENU_ITEM*>::iterator it = menu_items.begin(); it < menu_items.end(); it++){
		(*it)->Draw();
	}
	return true;
}
/* Display was forced - the bitmap is drawn on the screen. */
bool GAMEMENU::Display() {
	globals->ReCacheBitmap();
	StatusLog(L"SCR->DrawCachedBitmap(cached_bitmap, upper_left_corner_X, upper_left_corner_Y).", globals->GetScreenObject()->DrawCachedBitmap(globals->GetCachedBitmap(), upper_left_corner_X, upper_left_corner_Y), false);	
	return true;
}
/* Start game button was pressed - new game starts. */
bool GAMEMENU::GameStart() {
	// Recreates the controller, if nescessary
	globals->SetEnvironment(game);
	Log(L"\n... New game started ... ");
	FunctionLog(L"New game creation", globals->GetController()->CreateNewGame(), true); // Creates game
	return true;
}
/* Exit game button was pressed. */
bool GAMEMENU::ExitGame() {
	Log(L"\nGame exited.");
	PostQuitMessage(0);
	return true;
}
/* Continue button was pressed. */
bool GAMEMENU::Continue() {
	if(!globals->GetController()->IsGameCreated()) // New game wasn't created yet
		if(globals->GetController()->CreateGameFromSave()) // It is possible to create game from a save-file
			globals->GetController()->SceneAccesible(true);
		else
			return false;
	if(globals->GetController()->IsSceneAccesible()){
		globals->SetEnvironment(game);
		FunctionLog(L"From Continue menu button game Display call.", globals->GetController()->Display(), false);
		FunctionLog(L"From Continue menu button game Play call.", globals->GetController()->Play(), false);
	}
	else
		return false; // Scene is not accesible - it is impossible to continue
	return true;
}

int GAMEMENU::ChangeVolume(int pos, int width){
	float volume = (100.0f / static_cast<float>(width) )*static_cast<float>(pos); 
	globals->GetGameSettings()->SetValue(L"game_volume",static_cast<int>(volume));
	return static_cast<int>(volume);
}

/* Fullscreen button was pressed. */
void GAMEMENU::Fullscreen() {
	bool fullscreen = !(globals->GetGameSettings()->GetValue<bool>(L"fullscreen"));
	globals->GetGameSettings()->SetValue<bool>(L"fullscreen", fullscreen);

	Log(L"user-requested resolution change", DONT_TERM * MY_FUNC * LOG_FAIL, !globals->ChangeDisplayMode(window_width, window_height, globals->GetGameSettings()->GetValue<bool>(L"fullscreen")));	
	Display();

	globals->GetGameSettings()->SaveSettingsToFile();
}