#ifndef DEF_ENGINE_GAMECONTROL
#define DEF_ENGINE_GAMECONTROL

#include <string>
#include <fstream>
#include <dshow.h>
#include <D3D9.h>
#include <Vmr9.h>
#include "environment.h"

class SCENE;
class CMarkup;
class STATES;

class GAMECONTROL : public ENVIRONMENT {
private:
	// File-associated variables
	CMarkup* script;
	CMarkup* new_game;
	CMarkup* save_game;
	std::wofstream save;
	bool save_exists;
	// Game-associated variables
	SCENE*   work_scene;
	STATES*  game_states;
	// 
	std::wstring next_chapter;
	std::wstring next_scene;
	// System variables
	bool new_game_started; // True after game creation - blocks continuing from the savefile
	bool scene_accesible;  // If false, scene is currently building, or scene built failed
	bool game_finished;    // State at the end of the game
	int  volume;           // Volume of both the video and the sounds

public:
	GAMECONTROL();
	~GAMECONTROL();

	SCENE* GetScene(void) { return work_scene; }

	bool LoadStarting(); // Loads a game setting for the start
	bool LoadFromFile(); // Loads a game setting from the savefile
	void SaveToFile(std::wstring chapter, std::wstring scene); // Saves current game setting

	bool CreateNewGame(); // Creates first scene
	bool CreateGameFromSave(); // Creates first scene
	bool CreateNewScene(); // Creates scene according to the gameplay
	bool DetermineNext(); // Determines next chapter and scene
	bool SetPositionForNextScene(); // changes cursor position accordingly
	bool CreateScene(); // Builds scene from information on the current cursor postition	
	void SceneAccesible(const bool is_accesible) {scene_accesible = is_accesible;}
	bool IsSceneAccesible() {return scene_accesible;}
	bool DidGameFinished() {return game_finished;}	
	bool IsGameCreated() {return new_game_started;}
	bool CheckTests();

	bool Display(); // Response to the WM_PAINT message
	bool MouseMove(const int x, const int y);
	bool LeftClick(const int x, const int y);
	bool RightClick(const int x, const int y);
	bool Pause();
	bool Play();

	bool VideoEventOccured(); // Used when message for video event is recognized
	bool SubtitlesStart();
	bool SubtitlesEnd();
};

#endif