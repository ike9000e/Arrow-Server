
#ifndef _XEHI_LINUX_HELPER_H_
#define _XEHI_LINUX_HELPER_H_
#include <vector>
#include <string>
#include <X11/Xlib.h>
#include <stdint.h>

namespace hef{}
using namespace hef;

enum{
	/// State flags for \ref xehi_SetWindowNetWmStates().
	XEHI_SWS_AlwaysOnTopOn = 0x1,
	XEHI_SWS_AlwaysOnTopOff = 0x2,
	XEHI_SWS_SkipTaskbarOn = 0x4,
	XEHI_SWS_SkipTaskbarOff = 0x8,
	XEHI_SWS_AlwaysOnBottomOn = 0x10,
	XEHI_SWS_AlwaysOnBottomOff = 0x20,
	XEHI_SWS_SkipPagerOn = 0x40, ///< WIP. untested.
	//XEHI_SWS_SkipPagerOff = 0x80,
	XEHI_SWS_MaximizeOn = 0x100,  //WIP.
	XEHI_SWS_MaximizeOff = 0x200,
};
enum{
	/// Flags for \ref xehi_GetInputFocusWindow().
	/// if focus window itself does not have WM_CLASS attribute set,
	/// does not try to revert to 1st parent that has.
	XEHI_GIFW_FirstFound = 0x1,
};

enum{
	/// flags for \ref xehi_MessageBoxX11().
	XEHI_MB_CloseImmediatelly = 0x1,
	XEHI_MB_RefocusLastWnd = 0x2,
	XEHI_MB_RefocusLastWnd2 = 0x4,
	XEHI_MB_RefocusLWLowered = 0x8,
};
enum{
	XEHI_GATW_ByWmClass = 0x1,
};
/// Options for \ref xehi_MessageBoxX11() function.
struct XehiSMsgBoxX11{
	XehiSMsgBoxX11() : flags3(0), clrFrgg(0xFEFFFFFF), clrBckg(0xFEFFFFFF), calb2(0) {}
	XehiSMsgBoxX11( int flags_ ) : flags3(flags_), clrFrgg(0xFEFFFFFF), clrBckg(0xFEFFFFFF), calb2(0) {}
	int flags3;				///< flags fe. \ref XEHI_MB_CloseImmediatelly.
	unsigned long clrFrgg;
	unsigned long clrBckg; ///< background color fe. 0x00DCDAD5.
	void(*calb2)( Window wid, void* user2);
	void* user2;
};

void        xehi_MessageBoxX11( const char* title, const char* text, const XehiSMsgBoxX11& opt );
Window      xehi_GetAppTitleWindow( Display* dpy, Window wnd, int flags5=0 );
bool        xehi_SetWindowIcon( Display* dpy2, Window wnd, const unsigned long* pxs3, int num3, std::string* err2 );
std::string xehi_GetWindowName( Display* dpy, Window wnd );
void        xehi_SetWindowName( Display* dpy, Window wnd, const char* szNewName );
bool        xehi_MoveWindowToDesktop( Display* dpy, Window wid, int nDesktop );
int         xehi_GetNumberOfDesktops( Display* dpy );
bool        xehi_SetWindowPropertyAsText( Display* dpy, Window wid, const char* property2, const char* szVal );
bool        xehi_SetWindowPropertyAsLong( Display* dpy, Window wid, const char* property2, long value2 );
bool        xehi_SetWindowPropertyAsLongsList( Display* dpy, Window wid, const char* property2, const long* values2, int count2 );
uint8_t*    xehi_GetWindowProperty( Display* dpy, Window aWnd, Atom aPropType, const char* aPropName, unsigned long* nItems );
bool        xehi_GetWindowProperty2( Display* dpy, Window wnd, Atom atom2, int nBytesRetrieveLimit, std::vector<uint8_t>* outp, int* nbytes, int* nitemsOu, Atom* xaTypeOu, int* nItemSizeInBitsOu, std::string* err );
bool        xehi_GetWindowPropertyAsLong( Display* dpy, Window wid, const char* property2, long* valueOu, std::string* err );
std::string xehi_GetWindowPropertyAsText( Display* dpy, Window wid, const char* property2, std::string* err, int maxlen = -1 );
bool        xehi_IsWindow( Display* dpy, Window wid );
bool        xehi_SetWindowNetWmStates( Display* dpy, Window wid, int flags2 );
std::string xehi_GetAtomName( Display* dpy, Atom atom2 );
Window      xehi_GetInputFocusWindow( Display* dpy, int flags4=0, int* winPidOu=0 );
Window      xehi_GetParentWindow( Display* dpy, Window wid );

#endif //_XEHI_LINUX_HELPER_H_
