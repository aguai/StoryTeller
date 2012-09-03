#ifndef DEF_ENGINE_ENVIRONMENT
#define DEF_ENGINE_ENVIRONMENT

class ENVIRONMENT{
private:

	friend class GAMEMENU;
	friend class GAMECONTROL;

public:	

	ENVIRONMENT() {}
	ENVIRONMENT(const ENVIRONMENT & original) {}
	virtual ~ENVIRONMENT() = 0 {}

	// Functions that are common to both game and menu
	virtual bool MouseMove(const int x, const int y) {	return true;}
	virtual bool LeftClick(const int x, const int y) {	return true;}
	virtual bool RightClick(const int x, const int y) {	return true;}
	virtual bool Redraw() {	return true;}// Drawing to bitmap
	virtual bool Display() { return true;}// Displaying the bitmap
};

#endif