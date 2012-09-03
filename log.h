#ifndef DEF_ENGINE_FUNCTIONS
#define DEF_ENGINE_FUNCTIONS

///////////////////////////////////////////////////////////////////////////////////////////////////
// Codes for atributes of the log
// Behaviour codes
#define TERMINATE 2
#define DONT_TERM 3
#define LOG_ALL   5
#define LOG_FAIL  7
#define DONT_LOG  11
// Log type codes
#define MY_FUNC   13
#define DX_FUNC   17
#define API_FUNC  19
#define GDI_FUNC  23
// Log message prefix codes
#define LINE      29
#define TAB       31
#define STARS     37

// Log function
void Log(const std::wstring message, const long atributes, const long code);

	// For functions that returns DX HRESULT
void HRLog(const wchar_t * function, const long value, const bool terminate);
void HRLog(const wchar_t * function, const char * failure_message, const long value, const bool terminate);
	// For functions whose errors can be obtained through GetLastError
void APILog(const wchar_t * function, const int passed, const bool terminate);
void APILog(const wchar_t * function, const char * failure_message, const int passed, const bool terminate);
	// For GDIPlus functions
void StatusLog(const wchar_t * funkce, const Gdiplus::Status, const bool terminate);
void StatusLog(const wchar_t * funkce, const char * failure_message, const Gdiplus::Status, const bool terminate);
	// For my functions
void FunctionLog(const wchar_t * function, const int passed, const bool terminate);
void FunctionLog(const wchar_t * function, const wchar_t * failure_message, const int passed, const bool terminate);
	// Other universal logs
void TermLog(const wchar_t * zprava);
void Log(const unsigned int zprava);
void Log(const wchar_t * zprava);



#endif