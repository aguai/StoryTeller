#ifndef DEF_ENGINE_STATES
#define DEF_ENGINE_STATES

#include <map>

class CMarkup;

class STATES {
private:
	friend class SCENE;
	friend class GAMECONTROL;

	std::map<std::wstring, int> state_map; // Container for states
	std::map<std::wstring, int>::iterator state_pointer; // Pre-created (is used often)
public:	
	
	STATES();
	STATES(const STATES & original);
	STATES & operator=(const STATES & original);
	~STATES();
	bool ApplyChanges(const std::vector<state_changes> & changes); // Applies the changes on the state_map
};

#endif