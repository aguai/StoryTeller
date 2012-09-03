#include "header.h"
extern GLOBALS* globals;

// Nullifiing the values
SETTING::SETTING() {
	setting_file = nullptr;
}

// Specializations of template functions which a compiler should make
template bool SETTING::GetValue<bool>(std::wstring);
template int  SETTING::GetValue<int>(std::wstring);
template std::wstring SETTING::GetValue<std::wstring>(std::wstring);
template bool SETTING::SetValue<bool>(std::wstring, bool);
template bool SETTING::SetValue<int>(std::wstring, int);
template bool SETTING::SetValue<std::wstring>(std::wstring, std::wstring);

/**
 * Loads settings 
 * Format of settings file is:
 *    <ENTRY name="entry_name">entry_value</ENTRY> 
 */
bool SETTING::LoadSettings (void) {
	setting_file = new CMarkup;
	if (setting_file->Load(setting_file_name) == false) {
		std::wstring fail_msg;
		fail_msg.append(L"Loding of the file ").append(setting_file_name).append(L" failed with error ").append(setting_file->GetError());
		TermLog(fail_msg.c_str());
	}
	if (setting_file == nullptr) return false;

	setting_file->FindElem(L"SETTINGS");
	setting_file->IntoElem();

	std::wstring entry_name, entry_value;
	while (setting_file->FindElem(L"ENTRY")) {
		entry_name  = entry_value = L"error";
		entry_name  = setting_file->GetAttrib(L"name");
		entry_value = setting_file->GetData();
		settings.insert(SETVAL(entry_name, entry_value));
		if (!entry_name.compare(L"error") || !entry_value.compare(L"error")) {
			TermLog(L"Failed while read settings entry");
			return false;
		}
	}
	return true;
}

/**
 * Template Get-er for entry values - all used options must be specialized above
 */
template <class value_type>
value_type SETTING::GetValue(std::wstring name) {
	auto entry = settings.find(name);
	if (entry == settings.end()) {
		std::wstring output(std::wstring(L"Value ") + name + std::wstring(L" not found in the ") + setting_file_name + std::wstring(L"."));
		Log(output.c_str());
		return boost::lexical_cast<value_type, int>(0);
	}
	else
		return boost::lexical_cast<value_type, std::wstring>(entry->second);
}

/**
 * Template Set-er for entry values - all used options must be specialized above
 */
template <class value_type>
bool SETTING::SetValue(std::wstring name, value_type value) {
	auto entry = settings.find(name);
	if (entry != settings.end()) {
		entry->second = boost::lexical_cast<std::wstring, value_type>(value);
		return true;
	}
	else 
		return false;
}

/**
 * Saves the current setting to the file
 */
void SETTING::SaveSettingsToFile() {
	std::wofstream save;
	save.open(setting_file_name, std::ios_base::out|std::ios_base::trunc);

	save << L"<?xml version=\"1.0\"?>\n\n" << L"<SETTINGS>\n";
	for (auto entry = settings.begin(); entry != settings.end(); entry++)
		save << L"    <ENTRY name=\"" << entry->first << L"\">" << entry->second << L"</ENTRY>\n";
	save << L"</SETTINGS>";

	save.close();
}

