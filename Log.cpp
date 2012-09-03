#include <windows.h>
#include <fstream>
#include <DXErr.h>
#include "header.h"
extern GLOBALS* globals;

std::wofstream fout("log.txt"); // output file

/**
 * All-doing log function. The atrib property 
 * Implicite is no prefix, Log only on fail and don't terminate at all
 */
void Log(const std::wstring message, const long atrib, const long code) {
	long atributes = atrib;
	if ((atributes % LOG_ALL == 0) || ((atributes % DONT_LOG != 0) && ( code != 0))) {
		// Format prefix - if no prefix is specified, none is used
		while (atributes % TAB == 0) {
			fout << L"    ";
			atributes /= TAB;
		}
		while (atributes % LINE == 0) {
			fout << L"\n";
			atributes /= LINE;
		}
		while (atributes % STARS == 0) {
			fout << L"****";
			atributes /= STARS;
		}
		if (atributes % MY_FUNC == 0)
			fout << L"Engine function: " << message << L" failed with code " << code << L". Time: " << globals->GetTime() << L"." << std::endl;
		else if ((atributes % DX_FUNC == 0)) {
			std::wstring error_string = DXGetErrorString(code);
			fout << L"DirecX function: " << message << L" failed with error " << error_string << L". Time: " << globals->GetTime() << L"." << std::endl;
		}		
		else if ((atributes % API_FUNC == 0)) {
			long DX_code = HRESULT_FROM_WIN32(GetLastError());
			std::wstring error_string = DXGetErrorString(DX_code);
			fout << L"Win 32 function: " << message << L" failed. Last error is: " << error_string << L", Time: " << globals->GetTime() << L"." << std::endl;
		}
		else if ((atributes % GDI_FUNC == 0))
			fout << L"Gdiplus function: " << message << L" failed with status " << code << L". Time: " << globals->GetTime() << L"." << std::endl;
		else
			fout << message << L" Time: " << globals->GetTime() << L"." << std::endl;
	}

	if ((atributes % TERMINATE == 0) && ( code != 0)) {
			fout << L"TERMINATING" << std::endl;
			PostQuitMessage(code);
	}
}


/* Log for functions that returns HRESULT. */
void HRLog(const wchar_t * function, const long hr, const bool terminate){
	if(hr != S_OK){
		std::wstring error_string = DXGetErrorString(hr);

		fout << L"    Function: \"" << function << L"\" failed with result: " << error_string.c_str() << L" in time: " << globals->GetTime() << L" ms." << std::endl;
		if (terminate){
			fout << L"    Application will terminate." << std::endl;
			PostQuitMessage(hr);
		}
	}
}
/* Extended log for functions that returns HRESULT. */
void HRLog(const wchar_t * function, const char * failure_message, const long hr, const bool terminate){
	if(hr != S_OK){
		std::wstring error_string = DXGetErrorString(hr);

		fout << L"    Function: \"" << function << L"\" failed with result: " << error_string.c_str() << L" in time: " << globals->GetTime() << L" ms." << std::endl;
		fout << L"    Addtional message covering failure: " << failure_message << std::endl;
		if (terminate){
			fout << L"    Application will terminate." << std::endl;
			PostQuitMessage(hr);
		}
	}
}
/* Log for functions from WIN 32 API, whose failure message can be obtained by GetLastError. */
void APILog(const wchar_t * function, const int value, const bool terminate){
	if(value != 0){
		DWORD failure_report = GetLastError();

		fout << L"    API function: \"" << function << L"\" failed in time: " << globals->GetTime() << L" ms." << std::endl;
		fout << L"    Last error is: " << failure_report << L"." << std::endl;
		if(terminate){
			fout << L"    Application will terminate." << std::endl;
			PostQuitMessage(failure_report);
		}
	}
}
/* Extended log for functions from WIN 32 API, whose failure message can be obtained by GetLastError. */
void APILog(const wchar_t * function, const char * failure_message, const int value, const bool terminate){
	if(value != 0){
		DWORD failure_report = GetLastError();

		fout << L"    API function: \"" << function << L"\" failed with in time: " << globals->GetTime() << L" ms." << std::endl;
		fout << L"    Last error is: " << failure_report << L"." << std::endl;
		fout << L"    Addtional message covering failure: " << failure_message << std::endl;
		if(terminate){
			fout << L"    Application will terminate." << std::endl;
			PostQuitMessage(failure_report);
		}
	}
}
/* Log for my functions (return must be true). */
void FunctionLog(const wchar_t * function, const int passed, const bool terminate){
	if(passed == false){
		fout << L"    Function: \"" << function << L"\" failed in time: " << globals->GetTime() << L" ms." << std::endl;
		if(terminate){
			fout << L"    Application will terminate." << std::endl;
			PostQuitMessage(1);
		}
	}
}
/* Extended log for my functions (return must be true). */
void FunctionLog(const wchar_t * function, const wchar_t * failure_message, const int passed, const bool terminate){
	if(passed == false){
		fout << L"    Function: \"" << function << L"\" failed in time: " << globals->GetTime() << L" ms." << std::endl;
		fout << L"    Addtional message covering failure: " << failure_message << std::endl;
		if(terminate){
			fout << L"    Application will terminate." << std::endl;
			PostQuitMessage(1);
		}
	}
}
/* Log for functions that return status (return must be false). */
void StatusLog(const wchar_t * function, const Gdiplus::Status status, const bool terminate){
	if(status != 0){
		fout << L"   Function: \"" << function << L"\" failed in time: " << globals->GetTime() << L" ms." << std::endl;
		fout << L"   Gdiplus object status is: " << status << L"." << std::endl;
		if(terminate){
			fout << L"    Application will terminate." << std::endl;
			PostQuitMessage(status);
		}
	}	
}
/* Extended log for functions that return status (return must be false). */
void StatusLog(const wchar_t * function, const char * failure_message, const Gdiplus::Status status, const bool terminate){
	if(status != 0){
		fout << L"   Function: \"" << function << L"\" failed in time: " << globals->GetTime() << L" ms." << std::endl;
		fout << L"   Gdiplus object status is: " << status << L"." << std::endl;
		fout << L"   Addtional message covering failure: " << failure_message << std::endl;
		if(terminate){
			fout << L"   Application will terminate." << std::endl;
			PostQuitMessage(status);
		}
	}	
}
/* With-message terminal log. */
void TermLog(const wchar_t * message){
	fout << L"   Terminal failure: \"" << message << L"\" in time: " << globals->GetTime() << L" ms." << std::endl;
	PostQuitMessage(1);
}
/* Integer log. */
void Log(const unsigned int message){
	fout << L"   Message: " << message << L" = " << std::hex << L"0x" << message << std::dec << L" in time: " << globals->GetTime() << L"." << std::endl ;
}
/* Message log. */
void Log(const wchar_t * message){ // Engine reports
	fout << message <<  L" Time elapsed: " << globals->GetTime() << L"." << std::endl ;
}


/*

 HRESULT_FROM_WIN32 

typedef enum {
    Ok = 0,
    GenericError = 1,
    InvalidParameter = 2,
    OutOfMemory = 3,
    ObjectBusy = 4,
    InsufficientBuffer = 5,
    NotImplemented = 6,
    Win32Error = 7,
    WrongState = 8,
    Aborted = 9,
    FileNotFound = 10,
    ValueOverflow = 11,
    AccessDenied = 12,
    UnknownImageFormat = 13,
    FontFamilyNotFound = 14,
    FontStyleNotFound = 15,
    NotTrueTypeFont = 16,
    UnsupportedGdiplusVersion = 17,
    GdiplusNotInitialized = 18,
    PropertyNotFound = 19,
    PropertyNotSupported = 20,
    ProfileNotFound = 21
} Status;


*/