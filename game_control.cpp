// TODO: rewrite the constructor

#include "header.h"
extern GLOBALS* globals;

/**
 * Constructor, also prepares external files.
 */
GAMECONTROL::GAMECONTROL(){
	scene_accesible = game_finished = save_exists = new_game_started = false;
	work_scene      = nullptr;
	game_states     = nullptr;
	script          = new_game      = save_game   = nullptr;
	next_chapter    = next_scene    = L"";
	volume          = globals->GetGameSettings()->GetValue<int>(L"sound_volume");

	script = new CMarkup;
	if (script->Load(globals->GetGameSettings()->GetValue<std::wstring>(L"script_file")) == false) {
		std::wstring fail_msg;
		fail_msg.append(L"Loding of the file ").append(globals->GetGameSettings()->GetValue<std::wstring>(L"script_file")).append(L" failed with error ").append(script->GetError());
		TermLog(fail_msg.c_str());
	}

	new_game = new CMarkup;
		FunctionLog(L"Loading newgame.xml failed. File is probably missing." , new_game->Load(L"newgame.xml")  , true);
	// TO BE CHANGED (UNNESCESSARY TO FAIL
	save_game = new CMarkup;
		FunctionLog(L"Loading savegame.xml failed. File is probably missing.", save_game->Load(L"savegame.xml"), true);

	game_states = new STATES();
		FunctionLog(L"game_states allocation failed, probably out of memory", (game_states != nullptr), true);
	
	FunctionLog(L"Application couldn't find GAME tag in script.xml, file is probably damaged", script->FindElem(L"GAME"), true);

	script->IntoElem();
}

/**
 * Destructor.
 */
GAMECONTROL::~GAMECONTROL(){
	if(game_states != nullptr){
		delete game_states;
		game_states = nullptr;
	}
	if(work_scene != nullptr){
		delete work_scene;
		work_scene = nullptr;
	}
	if(script != nullptr){
		delete script;
		script = nullptr;
	}
	if(new_game != nullptr){
		delete new_game;
		new_game = nullptr;
	}
	if(save_game != nullptr){
		delete save_game;
		save_game = nullptr;
	}
}
/**
 * Function loads starting setting for a new game.
 */
bool GAMECONTROL::LoadStarting(){
	std::wstring state_name;
	std::wstring state_value;

	game_states->state_map.clear();

	new_game->ResetPos();

	if(!new_game->FindElem(L"SAVE")){
		Log(L"! <SAVE> tag missing.");
		return false;
	}
	new_game->IntoElem();

	if(!new_game->FindElem(L"CHAPTER")){
		Log(L"! <CHAPTER> tag missing.");
		return false;
	}
	next_chapter = new_game->GetData();

	if(!new_game->FindElem(L"SCENE")){
		Log(L"! <SCENE> tag missing.");
		return false;
	}
	next_scene = new_game->GetData();

	while(new_game->FindElem(L"STATE")){
		new_game->IntoElem();
		if(!new_game->FindElem(L"NAME")){
			Log(L"! <NAME> tag missing.");
			return false;
		}
		state_name = new_game->GetData();
		if(!new_game->FindElem(L"VALUE")){
			Log(L"! <VALUE> tag missing.");
			return false;
		}
		state_value = new_game->GetData();
		new_game->OutOfElem();

		game_states->state_map.insert(STATE(state_name, boost::lexical_cast<int,std::wstring>(state_value)));
	}
	new_game->OutOfElem();

	if (game_states->state_map.empty()){
		Log(L"! No states loaded.");
		return false;
	}
	return true;
}
/**
 * Loading of setting from previous game.
 */ 
bool GAMECONTROL::LoadFromFile(){
	std::wstring state_name;
	std::wstring state_value;

	save_game->ResetPos();

	if(!save_game->FindElem(L"SAVE")){
		Log(L"! <SAVE> tag missing.");
		return false;		
	}
	save_game->IntoElem();

	if(!save_game->FindElem(L"GAMESAVED")){
		Log(L"! <GAMESAVED> tag missing.");
		return false;		
	}
	if(save_game->GetData() != L"yes"){
		Log(L"Savefile is not used.");
		return false;		
	}
	
	game_states->state_map.clear();

	if(!save_game->FindElem(L"CHAPTER")){
		Log(L"! <CHAPTER> tag missing.");
		return false;
	}
	next_chapter = save_game->GetData();

	if(!save_game->FindElem(L"SCENE")){
		Log(L"! <SCENE> tag missing.");
		return false;
	}
	next_scene = save_game->GetData();

	while(save_game->FindElem(L"STATE")){
		save_game->IntoElem();
		if(!save_game->FindElem(L"NAME")){
			Log(L"! <NAME> tag missing.");
			return false;
		}
		state_name = save_game->GetData();
		if(!save_game->FindElem(L"VALUE")){
			Log(L"! <VALUE> tag missing.");
			return false;
		}
		state_value = save_game->GetData();
		save_game->OutOfElem();

		game_states->state_map.insert(STATE(state_name, boost::lexical_cast<int,std::wstring>(state_value)));
	}
	save_game->OutOfElem();

	if (game_states->state_map.empty()){
		Log(L"! No states loaded.");
		return false;
	}
	return true;
}

void GAMECONTROL::SaveToFile(std::wstring chapter, std::wstring scene){
	save.open("savegame.xml", std::ios_base::out|std::ios_base::trunc);
	std::wstring saved;
	if (DidGameFinished())
		saved.append(L"<GAMESAVED>no</GAMESAVED>\n");
	else
		saved.append(L"<GAMESAVED>yes</GAMESAVED>\n");
	save << L"<?xml version=\"1.0\"?>\n\n"
		 << L"<SAVE>\n"
		 << saved
		 << L"<CHAPTER>" << next_chapter << L"</CHAPTER>\n"
		 << L"<SCENE>"   << next_scene   << L"</SCENE>\n";

	game_states->state_pointer = game_states->state_map.begin();
	while(game_states->state_pointer != game_states->state_map.end()){
		save << L"<STATE>"
			 << L"<NAME>"  << game_states->state_pointer->first  << L"</NAME>"
			 << L"<VALUE>" << game_states->state_pointer->second << L"</VALUE>" 
			 << L"</STATE>"
			 << std::endl;
		game_states->state_pointer++;
	}
	save << L"</SAVE>\n";
	save.close();
}

/**
 * Creation of a new game - this function is called from the menu.
 */
bool GAMECONTROL::CreateNewGame(){
	script->ResetPos();
	FunctionLog(L"Application couldn't find GAME tag in script.xml, file is probably damaged", script->FindElem(L"GAME"), true);
	script->IntoElem();

	game_finished = false;

	if(!LoadStarting())
		return false;
	if(!SetPositionForNextScene())
		return false;
	if(!CreateScene())
		return false;
	new_game_started = true;
	return true;
}

/**
 * Creation of a new game from a savefile - this function is called from the menu.
 */
bool GAMECONTROL::CreateGameFromSave(){
	if(!LoadFromFile())
		return false;
	if(!SetPositionForNextScene())
		return false;
	if(!CreateScene())
		return false;
	new_game_started = true;
	return true;
}

/**
 * Setting the parser position in the .xml document.
 */
bool GAMECONTROL::SetPositionForNextScene(){
	std::wstring build_report = L"\n# Building chapter " + next_chapter + L", scene " + next_scene + L".";
	Log(build_report.c_str());

	std::wstring current_position = script->GetAttrib(L"no");		
	while(script->GetAttrib(L"no") != next_chapter){
		if(!script->FindElem(L"CHAPTER"))
			script->ResetMainPos(); // Resets the position within current bounds
		if(script->GetAttrib(L"no") == current_position){ // if the application makes a circle without effect, it terminates itself
			TermLog(L"Application couldn't find next chapter in script.xml, file is probably damaged");
			return false;
		}
	}
	script->IntoElem();

	current_position = script->GetAttrib(L"name");
	while(script->GetAttrib(L"name") != next_scene){
		if(!script->FindElem(L"SCENE"))
			script->ResetMainPos();  // Resets the position within current bounds
		if(script->GetAttrib(L"name") == current_position){
			TermLog(L"Application couldn't find next scene in script.xml, file is probably damaged");
			return false;
		}
	}
	script->IntoElem();	

	return true;
}

/**
 * Creating the new scene, called after old scene finished.
 */ 
bool GAMECONTROL::CreateNewScene(){
	if(!DetermineNext())
		return false;
	// Get from previous position
	script->OutOfElem();
	script->OutOfElem();

	if(SetPositionForNextScene())
		return CreateScene();
	else 
		return false;
}

/**
 * Finds out what the next scene should be.
 */
bool GAMECONTROL::DetermineNext(){
	/* Function  gets data from file and tests state variables against file tests*/
	FunctionLog(L"script->FindElem(L\"NEXT\")", L"Script file is damaged", script->FindElem(L"NEXT"), true);
	script->IntoElem();

	bool found = false;
	while(script->FindElem(L"CASE") && found != true){ // goes through all the cases
		script->IntoElem();
	
		if(CheckTests()){ // All tests passed
			if(!script->FindElem(L"NEXTCHAPTER"))
				TermLog(L"Application couldn't find next chapter in script.xml, file is probably damaged");
			next_chapter = script->GetData();

			if(!script->FindElem(L"NEXTSCENE"))
				TermLog(L"Application couldn't find next chapter in script.xml, file is probably damaged");
			next_scene = script->GetData();

			found = true;
		}
		script->OutOfElem();
	}

	if(found == false){ // No path passed - using deafault continue
		if(!script->FindElem(L"DEFAULT"))
			TermLog(L"Application couldn't find default continuing scene in script.xml, file is probably damaged");
		script->IntoElem();
		if(!script->FindElem(L"NEXTCHAPTER"))
			TermLog(L"Application couldn't find next chapter in script.xml, file is probably damaged");
		next_chapter = script->GetData();
		if(!script->FindElem(L"NEXTSCENE"))
			TermLog(L"Application couldn't find next scene in script.xml, file is probably damaged");
		next_scene = script->GetData();

		script->OutOfElem();
	}

	script->OutOfElem();

	return true;
}

/**
 * Building the scene at the position.
 */ 
bool GAMECONTROL::CreateScene(){	
	globals->GetScreenObject()->Clear(Gdiplus::Color(0,0,0)); // Covers what was left from previous session;
	
	if(work_scene != nullptr){
		delete work_scene;
		work_scene = nullptr;
	}

	FunctionLog(L"script->FindElem(L\"TYPE\")", L"Scene type couldn't be resolved, <TYPE> tag is missing.", script->FindElem(L"TYPE"), true);

	if(script->GetData() == L"video"){
		Log(L"    Creating VIDEO scene");		
		work_scene = new VIDEO(script, game_states);
	}
	else if(script->GetData() == L"dialogue"){
		Log(L"    Creating DIALOGUE scene");		
		work_scene = new DIALOGUE(script, game_states);
	}
	else if(script->GetData() == L"interactive"){
		Log(L"    Creating INTERACTIVE scene");		
		work_scene = new INTERACTIVE(script, game_states);	

	}
	else if(script->GetData() == L"finish"){
		Log(L"    Creating LAST VIDEO scene");
		work_scene = new VIDEO(script, game_states);
		game_finished = true;
	}

	if(work_scene == 0){
		TermLog(L"Application couldn't create next scene, file script.xml is probably damaged");
		return false;
	}

	// Save the game
	SaveToFile(next_chapter, next_scene);

	work_scene->SetVolume(volume);

	return true;
}

/**
 * Control of the tests for the next scene.
 */
bool GAMECONTROL::CheckTests(){
	std::wstring operation, state_name;
	int          value;

	bool fits = false;
	while(script->FindElem(L"TEST")){ // goes through all the tests
		operation = script->GetAttrib(L"op"); // Loads operator
		value = boost::lexical_cast<int, std::wstring>(script->GetAttrib(L"val")); // Makes value into number
		state_name = script->GetData();
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
			TermLog(output.c_str());
		} 

		if(fits == false)
			return false; // No need for other tests to go on
	}
	return true;
}

/**
 * Forced display of the current scene.
 */
bool GAMECONTROL::Display(){
	if ( scene_accesible )
		return work_scene->Repaint();
	else
		return false;
}

/**
 * Scene controll.
 */
bool GAMECONTROL::MouseMove(const int x, const int y){
	if ( scene_accesible )
		return work_scene->MouseMove(x,y);
	else
		return false;
}

/**
 * Scene controll.
 */
bool GAMECONTROL::LeftClick(const int x, const int y){
	if ( scene_accesible )
		return work_scene->LeftClick(x,y);
	else
		return false;
}

/**
 * Scene controll. 
 */
bool GAMECONTROL::RightClick(const int x, const int y){
	if ( scene_accesible )
		return work_scene->RightClick(x,y);
	else
		return false;
}

/**
 * Scene controll. 
 */
bool GAMECONTROL::Play(){
	if ( scene_accesible )
		return work_scene->Play();
	else
		return false;
}

/**
 * Scene controll. 
 */
bool GAMECONTROL::Pause(){
	if ( scene_accesible )
		return work_scene->Pause();
	else
		return false;
}

/**
 * Scene controll. 
 */
bool GAMECONTROL::VideoEventOccured(){
	if ( scene_accesible )
		return (S_OK == work_scene->EventResponse());
	else
		return false;
}

/**
 * SUBTITLES_STARTED response
 */
bool GAMECONTROL::SubtitlesStart() {
	if (! scene_accesible) {
		Log(L"Subtitle start called when scene was not acessible", TAB, 1);
		return false;
	}

	if (!(typeid(*work_scene) == (typeid(VIDEO)))) {
		Log(L"Subtitle start called for non-VIDEO scene", TAB, 1);
		return false;
	}

	Log(L"ScheduleNextSubtitles()", MY_FUNC * TAB, !dynamic_cast<VIDEO*>(work_scene)->ScheduleNextSubtitles());
	Log(L"DrawSubtitles(true)", MY_FUNC * TAB, !dynamic_cast<VIDEO*>(work_scene)->DrawSubtitles(true));
	return true;
}

/**
 * SUBTITLES_ENDED response
 */
bool GAMECONTROL::SubtitlesEnd() {
	if (! scene_accesible) {
		Log(L"Subtitle start called when scene was not acessible", TAB, 1);
		return false;
	}

	if (!(typeid(*work_scene) == (typeid(VIDEO)))) {
		Log(L"Subtitle start called for non-VIDEO scene", TAB, 1);
		return false;
	}

	Log(L"DrawSubtitles(false)", MY_FUNC * TAB, !dynamic_cast<VIDEO*>(work_scene)->DrawSubtitles(false));
	return true;
}