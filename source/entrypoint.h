#include <vector>
#include <string>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> //putenv()
#include <assert.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <dlfcn.h>
#include "xehi_linux_helper.h"

/*
	Selectors and actions provided by XWMODUTIL.
	Configuration must be done using the environment
	variable, name: XWMODUTIL_CFG. Items not marked as [x] are TODO, WIP or TBD.

	SELECTORS
	--------------
		[x] bOnFirstInput=1
		_       Action is activated by first user input. Key press,
		_       mouse button press or mouse move.
		[x] nSearchWndAfter=ms
		_       Timeout to search for target window the crude way.
		_       Value in milisecsonds (eg. 1000 is 1 second).
		[x] nUseMapFromTo=FROM,TO
		_       Action is performed on every N-th call to the XMapWindow(),
		_       while N >= FROM and N < TO.
		_       Eg. use 'nUseMapFromTo=3,4' to perform the actions when
		_       the process performs it's third call to the XMapWindow().
		_       XMapWindow() is the X11 funtion from the Xlib that programs must use
		_       to make every window they create visible.
		_       Using this API funtion, rather than one of create-window functions,
		_       seems to be more reliable.


	WINDOW ACTIONS
	----------------
		[x] mx2
		_       maximize window.
		[x] mi2
		_       minimize window.
		[x] exec2=c
		_       shell-command (tags: @pid@, @wid@, @dpy@. See note#2).
		[x] uwp2
		_       Update window pid property. The "_NET_WM_PID".
		[x] swi2=f
		_       Set window icon (see note#2).
		_       Image loading routine is very simple, only PAM images
		_       with RGBA pixel format are supported.
		_       Value must specify valid path to the PAM image.
		_       Example image size: 64x64.
		_       Command example, using Ffmpeg, to convert to comaptible format:
		_       $> ffmpeg -i ./a.png -an -f image2 -c:v pam -pix_fmt rgba ./a.pam
		[x] swg2=s
		_       Set window geometry (same as '-geometry' switch from Gnome
		_       environment (format: WxH+X+Y)).


	On PROCESS init ACTIONS
	----------------------------
		[x] pidfile=f
		_       Pid file name to create. Deleted on application exit (see note#2).
		[x] szActiveIf
		_       Optional condition that determines whenever XWMODUTIL should
		_       switch into dormant state. By default XWMODUTIL is in active state.
		_       A text that is to be matched on argv[0] of the current process to
		_       determine whenever XWMODUTIL should remain in active state.
		_       Simple text matching only.
		[x] bClearPreload
		_       If set, XWMODUTIL clears itself from LD_PRELOAD env var.
		_       Note: simple string matching, name of shared library cannot be changed.
		_       This action is performed only if not switched into dormant state
		_       (see 'szActiveIf').

	note#2:
	_   Percent-decoding where indicated. Encoding detection is by tail characters,
	_   ie. decoding is performed if the last 3 characters have been set to the
	_   '%HH' sequence, eg. '%20' stands for percent encoded space character.
	_   The '%00' can be used to make the last character removed from the result.

	TODO:
		[ ] Qt5 attach via QWindow::setTitle()
			--> .../08_taskbar_skip_dll_avidemux_related/entrypoint_dll.cpp

*/
struct SIntPair{
	int first, second;
};
struct SWhxy{
	int W,H,X,Y;
};
/// Structure for global variables.
/// Init shouldn't be done in C++ constructor, instead see \ref init2().
/// Member 'szActiveIf': simple strstr matching on arg0 only.
/// Must not contain any ':' character.
/// Note: std::pair<> seems to not initialize properly when in shared lib
/// _     as globals. not using containers from STL for global data.
struct SMSL{
	bool               bClearPreload;
	std::string*       szActiveIf;
	SIntPair           nUseMapFromTo;
	bool               bOnFirstInput;
	bool               mx2, mi2, bUpdWndPid;//mx2,mi2
	std::string*       exec2;
	int                nSearchWndAfter;
	std::string*       swi2;
	int                nInitedA, nInitedB;
	std::string*       szPidFile; //pidfile
	SWhxy              swg2;
	//
	static std::string toStr( const SMSL&, const char* tabs2 );
	void               init2();
	void               init3();
	bool isIntedAB()const;
};
/// The proceed-to-action struct.
struct SPTA {
	Display*   dpy;
	Window     wid;
	SPTA( Display* dpy_, Window wid_ ) : dpy(dpy_), wid(wid_) {}
};
enum{
	/// flags for app_LoadPAMRgbaImageSimple().
	HEF_PAMSL_PrependWH = 0x1,
	HEF_PAMSL_FormatRGBA = 0x2, // default.
	HEF_PAMSL_FormatARGB = 0x4, // note: Xlib|X11 cmaptible format.
	HEF_PAMSL_FormatBGRA = 0x8,
	HEF_PAMSL_FormatABGR = 0x10,
};

void                     app_ReadCfgFromEnv();
void                     app_SoLibInit() __attribute__((constructor));
void                     app_SoLibDeinit() __attribute__((destructor));
bool                     app_SetWindowIcon( Display* dpy, Window wnd, const unsigned long* pxs3, int nNumItems );
template<class T> bool   app_LoadPAMRgbaImageSimple( const char* szPAMFileName, int flags2, std::vector<T>& outp, std::string* err2, int* wdtOut, int* hgtOut );
std::string              app_SearchReplace( const char* inp, const char*const* srch, const std::vector<std::string>& repl2 );
std::string              app_StrVal( const int64_t inp, bool bFirst=1, uint64_t inp2=0 );
bool                     app_MaximizeWindow( Display* dpy, Window wid );
bool                     app_CreateThread( void*(*calb2)(void*), void* param2 );
bool                     app_GetCommandLineSOSpec( std::vector<std::string>& outp, uint32_t uPid );
bool                     app_SetWindowPropertyAsCardinal( Display* dpy, Window wid, const char* property2, unsigned long val );
void                     app_ProceedToAction( const SPTA& pta );
std::string              app_PercentDecodeIf( const char* inp, bool* bDecoded=0 );
std::vector<std::string> app_StrExplode( const char* inp, const char* glue2, int nLimit = -1 );
