#ifndef DEF_ENGINE_SETTING
#define DEF_ENGINE_SETTING

#include <map>

class SETTING {
private:
	CMarkup* setting_file;
	std::map<std::wstring, std::wstring> settings;

public:	
	SETTING();
	SETTING(const SETTING & original) { throw (L"SETTING copy constructor used"); }
	SETTING & operator=(const SETTING & original) { throw (L"SETTING assign operator used"); }
	~SETTING();

	bool LoadSettings (void);
	void SaveSettingsToFile(void);

	template <class value_type>
		value_type GetValue(std::wstring name);
	template <class value_type>
		bool SetValue(std::wstring name, value_type value);
};

#endif