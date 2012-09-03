#include <fstream>
#include "header.h"
extern GLOBALS* globals;

STATES::STATES(){
}

STATES::STATES(const STATES & original){
	state_map = original.state_map;
	state_pointer = original.state_pointer;
}

STATES & STATES::operator=(const STATES & original){
	state_map = original.state_map;
	state_pointer = original.state_pointer;
	return *this;
}

STATES::~STATES(){
	state_map.clear();
}
/* Takes a vector of changes and applies them to the state map. */
bool STATES::ApplyChanges(const std::vector<state_changes> & changes){
	for(std::vector<state_changes>::const_iterator change = changes.begin(); change < changes.end(); change++){
		state_pointer = state_map.find(change->name);
		if(state_pointer == state_map.end()){ // state not found
			std::wstring fail_msg(change->name);
			fail_msg.append(L": state was not found in the state holder");
			Log(fail_msg.c_str());
			continue;
		}
		else{
			if(change->replace)
				state_pointer->second =  change->value;
			else
				state_pointer->second += change->value;
		}
	}
	return true;
}
