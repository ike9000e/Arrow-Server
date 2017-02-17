
#include "xehi_linux_helper.h"
#include <cstring>      //strlen()
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <limits.h>     //LONG_MAX
#include <stdio.h>
#include <unistd.h>

/// \cond DOXYGEN_SKIP //[QJj9NJ38o]
;
typedef struct {
	int x, y;
	unsigned int width, height;
	int textx, texty;
	int mouseover;
	int clicked;
	const char* text;
} _xehi_button;

bool xehi_IsWindow2( Display* dpy, Window widParent, Window wndNeedle );

static void _xehi_draw_button( _xehi_button* b, int fg, int bg,
						 Display* dpy, Window w, GC gc )
{
	if( b->mouseover ){
		XFillRectangle( dpy, w, gc, b->clicked+b->x, b->clicked+b->y,
									b->width, b->height );
		XSetForeground( dpy, gc, bg );
		XSetBackground( dpy, gc, fg );
	}
	else
	{
		XSetForeground( dpy, gc, fg );
		XSetBackground( dpy, gc, bg );
		XDrawRectangle( dpy, w, gc, b->x, b->y, b->width, b->height );
	}

	XDrawString( dpy, w, gc, b->clicked+b->textx, b->clicked+b->texty,
				 b->text, strlen(b->text) );
	XSetForeground( dpy, gc, fg );
	XSetBackground( dpy, gc, bg );
}

static int _xehi_is_point_inside( _xehi_button* b, int px, int py )
{
	return px>=b->x && px<=(b->x+(int)b->width-1) &&
			py>=b->y && py<=(b->y+(int)b->height-1);
}

bool xehi_IsWindow2( Display* dpy, Window widParent, Window wndNeedle )
{
	Window rootw = 0, parentw = 0, *childrenw = 0; unsigned int nchi = 0, i;
	if( !XQueryTree( dpy, widParent, &rootw, &parentw, &childrenw, &nchi ) )
		return 0;
	if( childrenw ){
		bool res = 0;
		for( i=0; i<nchi; i++ ){
			Window wid = childrenw[i];
			if( wid && (wid == wndNeedle || xehi_IsWindow2( dpy, wid, wndNeedle )) ){
				res = 1;
				break;
			}
		}
		XFree( childrenw );
		childrenw = 0;
		return res;
	}
	return 0;
}

/// \endcond //DOXYGEN_SKIP //[QJj9NJ38o]
;

/*
//#define WM_TYPE_PROP "_NET_WM_WINDOW_TYPE"
//#define WM_TYPE_DESKTOP_PROP "_NET_WM_WINDOW_TYPE_DESKTOP"
bool _xehi_isVirtualRoot( Display* mDisplay, Window hWin )
{
	unsigned char *windowTypeRaw = NULL;
	Atom *windowType;
	unsigned long ulCount;
	bool rs2 = 0;

	//LogRelFlowFunc(("\n"));
	windowTypeRaw = xehi_GetWindowProperty( mDisplay, hWin, XA_ATOM, "_NET_WM_WINDOW_TYPE", &ulCount );
	if( windowTypeRaw != NULL ){
		windowType = (Atom *)(windowTypeRaw);
		if( (ulCount != 0) && (*windowType == XInternAtom(mDisplay, "_NET_WM_WINDOW_TYPE_DESKTOP", 1 )) )
			rs2 = 1;
	}
	if (windowTypeRaw)
		XFree(windowTypeRaw);
	//LogRelFlowFunc(("returning %RTbool\n", rs2));
	return rs2;
}//*/


/**
	Retrieves title window of specified input window.

	Usefull fe. to fully bring to screen and activate window of an another application.
	Having retrieved previously active window with XGetInputFocus(), retrieve it's title
	window with this function, then:
	* use previously active window with XSetInputFocus() to make input active on it,
	* use title window with XRaiseWindow() to activate the application.

	\param flags5 - flags, fe. \ref XEHI_GATW_ByWmClass.
*/
Window xehi_GetAppTitleWindow( Display* dpy, Window wnd, int flags5 )
{
	if( !(flags5 & XEHI_GATW_ByWmClass) ){
		Window rootw = 0, parentw = 0, *childrenw = 0; unsigned int nchi = 0;
		rootw = DefaultRootWindow(dpy);
		if( rootw == wnd )
			return 0;
		for( ;; ){
			parentw = rootw = 0;
			if( !XQueryTree( dpy, wnd, &rootw, &parentw, &childrenw, &nchi ) )
				return 0;
			if(childrenw){
				XFree( childrenw );
				childrenw = 0;
			}
			if( rootw != parentw && parentw && rootw ){
				wnd = parentw;
				continue;
			}
			break;
		}
		return wnd;
	}else{
		// old: app_ToTopParentWindow()
		std::string str;
		for(; wnd; wnd = xehi_GetParentWindow(dpy,wnd) ){
			str = xehi_GetWindowPropertyAsText( dpy, wnd, "WM_CLASS", 0, 5 );
			//printf("MAXIMIZESOLIB: WM_CLASS: prop:[%s] %d, t:[%s]\n", str.c_str(), (int)wnd, xehi_GetWindowName(dpy,wnd).c_str() );
			if( !str.empty() )
				break;
		}
		return wnd;
	}
}
/**
	A "small" and "simple" function that creates a message box with an OK
	button, using ONLY Xlib.
	The function does not return until the user closes the message box,
	using the OK button, the escape key, or the close button what means
	that you can't do anything in the mean time(in the same thread)
	The code may look very ugly, because I pieced it together from
	tutorials and manuals and I use an awfull lot of magic values and
	unexplained calculations.

	\param title - The title of the message box.
	\param text  - The contents of the message box. Use '\n' as a line terminator.
	\param opt - options structure. Provides flags adaptor
				 constructor, single integer argument. See XehiSMsgBoxX11().

	\n

	X11 Message Box functions.
	-------------------------------------------
	source link:
	http://www.google.pl/url?sa=t&rct=j&q=&esrc=s&source=web&cd=2&ved=0CCgQFjABahUKEwiJivDxnvvGAhXr6nIKHTyNBww&url=http%3A%2F%2Flibgoliath.so%2Fsw%2FMessageBoxX11.c&ei=hhq2VYmvE-vVywO8mp5g&usg=AFQjCNFc1bBLBGiIFVDmoaaKGjF3qMekLA&bvm=bv.98717601,d.bGQ&cad=rja

	Original author note:
	\verbatim
		//   I, David Oberhollenzer, author of this file hereby place the contents of
		//   this file into the public domain. Please feel free to use this file in any
		//   way you wish.
		//   I want to do this, because a lot of people are in the need of a simple X11
		//   message box function.
		//   The original version was written in C++ and has been ported to C. This
		//   version is entirely leak proof! (According to valgrind)
	\endverbatim
*/
void xehi_MessageBoxX11( const char* title, const char* text, const XehiSMsgBoxX11& opt  )
{
	const char* wmDeleteWindow = "WM_DELETE_WINDOW";
	int black, white, height = 0, direction, ascent, descent, X, Y, W=0, H;
	size_t i, lines = 0;
	char *atom;
	const char *end, *temp;
	_xehi_button ok;
	Display* dpy;
	Window w, wFocusOvr;
	Atom wmDelete;
	GC gc;
	XFontStruct* font;
	XCharStruct overall;
	XSizeHints hints;
	XEvent e;

	//Open a display
	if( !(dpy = XOpenDisplay( 0 )) )
		return;

	if( opt.flags3 & (XEHI_MB_RefocusLastWnd | XEHI_MB_RefocusLastWnd2) ){
		int dmy2;
		wFocusOvr = 0;
		XGetInputFocus( dpy, &wFocusOvr, &dmy2 );
	}
	// Get us a white and black_ color
	black = ( opt.clrFrgg == 0xFEFFFFFF ? BlackPixel( dpy, DefaultScreen(dpy) ) : opt.clrFrgg );
	white = ( opt.clrBckg == 0xFEFFFFFF ? WhitePixel( dpy, DefaultScreen(dpy) ) : opt.clrBckg );

	// Create a window with the specified title
	// w = XCreateSimpleWindow( dpy, DefaultRootWindow(dpy), 0, 0, 100, 100,
	//                         0, black, black );
	w = XCreateSimpleWindow( dpy, DefaultRootWindow(dpy), 0, 0, 100, 100,
							 0, white, white );


	XSelectInput( dpy, w, ExposureMask | StructureNotifyMask |
						  KeyPressMask | KeyReleaseMask | PointerMotionMask |
						  ButtonPressMask | ButtonReleaseMask | FocusChangeMask
							);

	XMapWindow( dpy, w );
	XStoreName( dpy, w, title );

	wmDelete = XInternAtom( dpy, wmDeleteWindow, 1 );
	XSetWMProtocols( dpy, w, &wmDelete, 1 );

	// Create a graphics context for the window
	gc = XCreateGC( dpy, w, 0, 0 );

	XSetForeground( dpy, gc, black );
	XSetBackground( dpy, gc, white );

	// Compute the printed width and height of the text
	if( !(font = XQueryFont( dpy, XGContextFromGC(gc) )) )
		goto cleanup;

	for( temp=text; temp; temp = end ? (end+1) : NULL, ++lines ){
		end = strchr( temp, '\n' );

		XTextExtents( font, temp, end ? (unsigned int)(end-temp):strlen(temp),
					  &direction, &ascent, &descent, &overall );

		W = overall.width>W ? overall.width : W;
		height = (ascent+descent)>height ? (ascent+descent) : height;
	}

	// Compute the shape of the window and adjust the window accordingly
	W += 20;
	H = lines*height + height + 40;
	W = std::max( W, 200 );
	X = DisplayWidth ( dpy, DefaultScreen(dpy) )/2 - W/2;
	Y = DisplayHeight( dpy, DefaultScreen(dpy) )/2 - H/2;

	XMoveResizeWindow( dpy, w, X, Y, W, H );

	// Compute the shape of the OK button
	XTextExtents( font, "OK", 2, &direction, &ascent, &descent, &overall );

	ok.width = overall.width + 30;
	ok.height = ascent + descent + 5;
	ok.x = W/2 - ok.width/2;
	ok.y = H   - height - 15;
	ok.textx = ok.x + 15;
	ok.texty = ok.y + ok.height - 3;
	ok.mouseover = 0;
	ok.clicked = 0;
	ok.text = "OK";

	XFreeFontInfo( NULL, font, 1 ); // We don't need that anymore

	// Make the window non resizeable
	//XUnmapWindow( dpy, w );

	hints.flags      = PSize | PMinSize ;//| PMaxSize ;//| USPosition;
	hints.min_width  = hints.max_width  = hints.base_width  = W;
	hints.min_height = hints.max_height = hints.base_height = H;
	//hints.x = hints.y = 0;

	XSetWMNormalHints( dpy, w, &hints );
	//XMapRaised( dpy, w );
	XMapWindow( dpy, w );
	XFlush( dpy );

	if( (opt.flags3 & XEHI_MB_RefocusLastWnd) && wFocusOvr ){
		//printf("INFO: using XWindowChanges struct.\n");
		Window wnd2 = xehi_GetAppTitleWindow( dpy, wFocusOvr );
		if( wnd2 ){
			XSetInputFocus( dpy, wFocusOvr, RevertToParent, CurrentTime );
			XRaiseWindow( dpy, wnd2 );
			if( opt.flags3 & XEHI_MB_RefocusLWLowered )
				XLowerWindow( dpy, w );
		}
	}
	if( opt.calb2 )
		opt.calb2( w, opt.user2 );
	for(;;){ // Event loop
		XNextEvent( dpy, &e );
		ok.clicked = 0;

		if( e.type == MotionNotify ){
			if( _xehi_is_point_inside( &ok, e.xmotion.x, e.xmotion.y ) ){
				if( !ok.mouseover )
					e.type = Expose;
				ok.mouseover = 1;
			}else{
				if( ok.mouseover )
					e.type = Expose;
				ok.mouseover = 0;
				ok.clicked = 0;
			}
		}
		switch( e.type ){
		case FocusIn:
			//printf("FocusIn event.\n");
			if( wFocusOvr && ( opt.flags3 & XEHI_MB_RefocusLastWnd2 ) ){
				//printf("FocusIn event, foc.\n");
				Window wnd2 = xehi_GetAppTitleWindow( dpy, wFocusOvr );
				if( wnd2 ){
					XSetInputFocus( dpy, wFocusOvr, RevertToParent, CurrentTime );
					XRaiseWindow( dpy, wnd2 );
					if( opt.flags3 & XEHI_MB_RefocusLWLowered )
						XLowerWindow( dpy, w );
				}
				wFocusOvr = 0;
			}
			{
			//	wFocusOvr = 0;
			//	int dmy2;
			//	XGetInputFocus( dpy, &wFocusOvr, &dmy2 );
			//	if( w != wFocusOvr && wFocusOvr ){
			//		Window wnd2 = xehi_GetAppTitleWindow( dpy, wFocusOvr );
			//		if( wnd2 ){
			//			XSetInputFocus( dpy, wFocusOvr, RevertToParent, CurrentTime );
			//			XRaiseWindow( dpy, wnd2 );
			//		}
			//		usleep(100000);
			//	}
			}
			break;
		case ButtonPress:
		case ButtonRelease:
			if( e.xbutton.button != Button1 )
				break;

			if( ok.mouseover )
			{
				ok.clicked = e.type==ButtonPress ? 1 : 0;

				if( !ok.clicked )
					goto cleanup;
			}
			else
			{
				ok.clicked = 0;
			}

		case Expose:
		case MapNotify:
			XClearWindow( dpy, w );
			// Draw text lines
			for( i=0, temp=text; temp; temp=end ? (end+1) : NULL, i+=height ){
				end = strchr( temp, '\n' );
				XDrawString( dpy, w, gc, 10, 10+height+i, temp,
							 end ? (unsigned int)(end-temp) : strlen(temp) );
			}
			// Draw OK button
			_xehi_draw_button( &ok, black, white, dpy, w, gc );
			XFlush( dpy );
			break;
		case KeyRelease: {
				int ksm = XLookupKeysym( &e.xkey, 0 );
				if( ksm == XK_Escape || ksm == XK_Return || ksm == 0x20 || ksm == XK_KP_Enter )
					goto cleanup;
			}
			break;
		case ClientMessage:
			atom = XGetAtomName( dpy, e.xclient.message_type );
			//if( *atom == *wmDeleteWindow )
			if( !strcmp( atom, wmDeleteWindow ) ){
				XFree( atom );
				goto cleanup;
			}
			XFree( atom );
			break;
		}
		if( opt.flags3 & XEHI_MB_CloseImmediatelly ){
			goto cleanup;
		}
	}
cleanup:
	XFreeGC( dpy, gc );
	XDestroyWindow( dpy, w );
	XCloseDisplay( dpy );
}

/**
    Sets X11 window icon provided pixels as the 'unsigned long' array.
    \verbatim
		Note: it is important that array is made of actual "unsigned long" elements
			  as this type has different size depending on the architecture,
			  fe. "unsigned long" is a 64-bit type (8-bytes) on x86_64 and
			  32-bit (4-bytes) on x86-s.
	\endverbatim
	First two elements in the 'pxs3' array must be width and height.

	Example:
	\code
		std::string err, fnm = "./mainicon.tga";
		std::vector<unsigned long> pxs2;
		if( !xehi_LoadTgaIconAsUlongs( fnm->c_str(), pxs2, &err ) ){
			printf("ERROR: icon load failed [%s]\n", fnm->c_str() );
			printf("Message: [%s]\n", err.c_str() );
		}else{
			xehi_SetWindowIcon( dpy2, wnd, &pxs2[0], pxs2.size(), &err );
		}
	\endcode
*/
bool xehi_SetWindowIcon( Display* dpy2, Window wnd, const unsigned long* pxs3, int num3,
						std::string* err2 )
{
	std::string err3, *err = ( err2 ? err2 : &err3 );
	if( !num3 ){
		*err = "Empty pixels provided.";
		return 0;
	}
	Atom property2 = XInternAtom( dpy2, "_NET_WM_ICON", 0 );
	if(!property2){
		*err = "Property _NET_WM_ICON null.";
		return 0;
	}
	// pixel format for XChangeProperty() call: 0xAARRGGBB.
	// (on 64-bit sustem: 0x00000000AARRGGBB (note: unsigned long is 64-bit on x86_64)).
	int res = XChangeProperty( dpy2, wnd, property2, XA_CARDINAL, 32,
				PropModeReplace,
				//(const unsigned char*)&pxs3[0], pxs3.size()
				(const unsigned char*) pxs3, num3 );
	if( !res ){
		*err = "XChangeProperty failed.";
		return 0;
	}
	res = XFlush( dpy2 );
	if( !res ){
		*err = "XFlush failed.";
		return 0;
	}
	return 1;
}
std::string xehi_GetWindowName( Display* dpy, Window wnd )
{
	char* nme = 0;
	if(wnd){
		XFetchName( dpy, wnd, &nme );
		if( nme )
			return nme;
	}
	return "";
}
void xehi_SetWindowName( Display* dpy, Window wnd, const char* szNewName )
{
	XStoreName( dpy, wnd, szNewName );
}
/*
/// Retrieves name of file Window manager configuration is stored in.
/// Uses "_OB_CONFIG_FILE" window property (Window managers that adhere to
/// the FreeDesktop standards).
/// Eg: "/home/john/.config/openbox/lubuntu-rc.xml".
std::string xehi_GetDesktopConfigFileName( Display* dpy )
{
	Window rootw = DefaultRootWindow(dpy);
	if(rootw){
		unsigned long ulCount = 0;
		uint8_t* propValue = 0;
		Atom atm2 = XInternAtom( dpy, "UTF8_STRING", 0 ); //1=only if exists.
		if(!atm2)
			return "";
		propValue = xehi_GetWindowProperty( dpy, rootw, atm2,
					"_OB_CONFIG_FILE", &ulCount );
		std::string str( (const char*)propValue );
		//printf("ptr:%d [%s], cnt:%d\n",
		//		(int)(size_t)propValue, str.c_str(),
		//		(int)ulCount );
		if(propValue)
			XFree(propValue);
		return str;
	}
	return "";
}//*/

/// Moves window to specified desktop.
/// \verbatim
/// ref: xdotool
///      function: xdo_set_desktop_for_window( const xdo_t *xdo, .... )
///      file: ".../xdotool_3x20130111x1x3x1_a64/src/u1/xdo.c"
/// \endverbatim
/// Root window can be queried for number of available desktops.
/// It's property name "_NET_NUMBER_OF_DESKTOPS" holds actual number as long type.
/// \sa xehi_GetNumberOfDesktops()
bool xehi_MoveWindowToDesktop( Display* dpy, Window wid, int nDesktop )
{
	XEvent xev;
	XWindowAttributes wattr;
	XGetWindowAttributes( dpy, wid, &wattr );
	memset( &xev, 0, sizeof(xev) );
	xev.type = ClientMessage;
	xev.xclient.display = dpy;
	xev.xclient.window = wid;
	xev.xclient.message_type = XInternAtom( dpy, "_NET_WM_DESKTOP", 0 );
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = (long)nDesktop;
	xev.xclient.data.l[1] = 2; // indicate we are messaging from a pager.

	XSendEvent( dpy, wattr.screen->root, 0,
				SubstructureNotifyMask | SubstructureRedirectMask,
				&xev );
	return 1;
}
int xehi_GetNumberOfDesktops( Display* dpy )
{
	long num = 0;
	Window rootw = DefaultRootWindow(dpy);
	if(rootw){
		if( xehi_GetWindowPropertyAsLong( dpy, rootw, "_NET_NUMBER_OF_DESKTOPS", &num, 0 ) )
			return num;
	}
	return 0;
}
// old: _xehi_XXGetProperty()
/// Returns window property.
/// Returned data must be freed using XFree().
uint8_t* xehi_GetWindowProperty( Display* dpy, Window aWnd, Atom aPropType,
									const char* aPropName, unsigned long* nItems )
{
	unsigned long nItems3, *nItems2 = ( nItems ? nItems : &nItems3 );
	Atom propNameAtom = XInternAtom( dpy, aPropName, 1 ); // 1=only_if_exists
	if( propNameAtom == None ){
		return NULL;
	}
	Atom actTypeAtom = None;
	int actFmt = 0;
	unsigned long nBytesAfter = 0;
	unsigned char* propVal = 0;
	int rc = XGetWindowProperty( dpy, aWnd, propNameAtom,
								 0, LONG_MAX, 0, // 0=delete
								 aPropType, &actTypeAtom, &actFmt,
								 nItems2, &nBytesAfter, &propVal );
	if( rc != Success )
		return 0;
	return propVal;
}
bool xehi_SetWindowPropertyAsLong( Display* dpy, Window wid, const char* property2, long value2 )
{
	// PropModeReplace, PropModePrepend, or PropModeAppend.
	int rs2;
	rs2 = XChangeProperty( dpy, wid,
						XInternAtom( dpy, property2, 0 ),
						XInternAtom( dpy, "CARDINAL", 0 ), 32,
						PropModeReplace,
						//PropModeAppend,
						(unsigned char*)&value2, 1 );
	return !rs2;
}
bool xehi_SetWindowPropertyAsLongsList( Display* dpy, Window wid, const char* property2, const long* values2, int count2 )
{
	int rs2;
	rs2 = XChangeProperty( dpy, wid,
						XInternAtom( dpy, property2, 0 ),
						XInternAtom( dpy, "CARDINAL", 0 ), 32,
						PropModeReplace, (unsigned char*)values2, count2 );
	return !rs2;
}
bool xehi_SetWindowPropertyAsAtomList( Display* dpy, Window wid, const char* property2, const Atom* values2, int count2 )
{
	int rs2; // PropModeReplace, PropModePrepend, or PropModeAppend.
	rs2 = XChangeProperty( dpy, wid,
						XInternAtom( dpy, property2, 0 ),
						XA_ATOM, sizeof(Atom)*8,
						PropModePrepend, (unsigned char*)values2, count2 );
	return !rs2;
}

bool xehi_SetWindowPropertyAsText( Display* dpy, Window wid, const char *property, const char *szVal )
{
	// shell xprop examples:
	// _NET_WM_PID(CARDINAL) = 8775
	// _OB_APP_CLASS(UTF8_STRING) = "XTerm"
	// WM_LOCALE_NAME(STRING) = "en_US.UTF-8"
	int rs2 = 0;
	rs2 = XChangeProperty( dpy, wid,
						XInternAtom( dpy, property, 0 ),
						XInternAtom( dpy, "UTF8_STRING", 0 ), 8,//STRING,UTF8_STRING
						PropModeReplace, (unsigned char*)szVal, strlen(szVal) );
	return !rs2;
}

// xdotool:
// // Arbitrary window property retrieval
// // slightly modified version from xprop.c from Xorg
// // xdo_get_window_property_by_atom(const xdo_t *xdo, .... )
// // --> ".../xdotool_3x20130111x1x3x1_a64/src/u1/xdo.c"
bool xehi_GetWindowProperty2( Display* dpy, Window wnd, Atom atom2,
									int nBytesRetrieveLimit,
									std::vector<uint8_t>* outp,
									int* nbytes,
									int* nitemsOu, Atom* xaTypeOu,
									int* nItemSizeInBitsOu,
									std::string* err )
{
	std::string err3, &err2 = ( err ? *err : err3 );
	Atom actual_type;
	int actual_format, status2;
	long nbytes2 = 0;
	unsigned long nitems2, bytes_after; //unused.
	unsigned char* prop;
	long long_length = ( nBytesRetrieveLimit != -1 ? nBytesRetrieveLimit/4 + !!(nBytesRetrieveLimit%4) : (~0L) );
	status2 = XGetWindowProperty( dpy, wnd,
					atom2,
					//XInternAtom( dpy, property2, 0 ),
					0, long_length, //(~0L)
					0, AnyPropertyType, &actual_type,
					&actual_format, &nitems2, &bytes_after,
					&prop );
	if( status2 == BadWindow ){
		char bfr[128];
		sprintf( bfr, "Window id %d does not exists.", (int)wnd );
		err2 = bfr;
		return 0;
	}
	if( status2 != Success ){
		err2 = "XGetWindowProperty() failed!";
		return 0;
	}
	if (actual_format == 32){
		nbytes2 = nitems2 * sizeof(long);
	}else if (actual_format == 16){
		nbytes2 = nitems2 * sizeof(short);
	}else if (actual_format == 8){
		nbytes2 = nitems2 * sizeof(char);
	}else{
		err2 = "Unknown data format returned by XGetWindowProperty().";
		return 0;
	}
	if( nitemsOu )
		*nitemsOu = (int)nitems2;
	if( xaTypeOu )
		*xaTypeOu = actual_type;
	if( nItemSizeInBitsOu )
		*nItemSizeInBitsOu = actual_format;
	if( nbytes )
		*nbytes = (int)nbytes2;
	if(outp){
		int nbytes3 = ( nBytesRetrieveLimit != -1 ? nBytesRetrieveLimit : (int)nbytes2 );
		outp->resize( nbytes3, 0 );
		memcpy( &(*outp)[0], prop, nbytes3 );
	}
	XFree(prop);
	return 1;
}
bool xehi_GetWindowPropertyAsLong( Display* dpy, Window wid, const char *property2, long* valueOu, std::string* err )
{
	std::string err3, &err2 = ( err ? *err : err3 );
	std::vector<uint8_t> data2; bool rs2; int nBitsPerItem = 0;
	*valueOu = 0;
	rs2 = xehi_GetWindowProperty2( dpy, wid,
					XInternAtom( dpy, property2, 0 ),
					-1, &data2, 0,0,0, &nBitsPerItem, &err2 );
	if(!rs2)
		return 0;
	if( data2.size() < sizeof(long) ){
		err2 = "Data returned too small (size < sizeof(long)).";
		return 0;
	}
	*valueOu = *((long*)( &data2[0] ));
	//printf("Got %d bytes, nBitsPerItem:%d, sizeofl:%d, lng:%d, err:[%s].\n",
	//		(int)data2.size(), nBitsPerItem, (int)sizeof(long), (int)(*valueOu), err2.c_str() );
	return 1;
}
/// Returns property value as c-string (null-terminated string).
/// Text can be assumed to be UTF-8.
/// Can return useless data if property was not an array of
/// null-terminated 8-bit characters.
/// \param maxlen - set to -1 to ignore, can be used to limit num of characters to retrieve.
std::string xehi_GetWindowPropertyAsText( Display* dpy, Window wid, const char* property2, std::string* err, int maxlen )
{
	std::string str, err3, &err2 = ( err ? *err : err3 );
	std::vector<uint8_t> data2; bool rs2; char chr;
	rs2 = xehi_GetWindowProperty2( dpy, wid,
					XInternAtom( dpy, property2, 0 ),
					maxlen, &data2, 0,0,0,0, &err2 );
	if(!rs2)
		return "";
	for( size_t i=0; i<data2.size(); i++ ){
		if( !(chr = (char)data2[i]) )
			break;
		str += chr;
	}
	return str;
}
bool xehi_IsWindow( Display* dpy, Window widNeedle )
{
	Window rootw = DefaultRootWindow(dpy);
	if( rootw == widNeedle )
		return 1;
	return xehi_IsWindow2( dpy, rootw, widNeedle );
}

/// Sets window states.
/// Xlib atom name is "_NET_WM_STATE".
/// \param flags2 - flags, fe. \ref XEHI_SWS_AlwaysOnTopOn.
bool xehi_SetWindowNetWmStates( Display* dpy, Window wid, int flags2 )
{
	Atom xaNetWmState = XInternAtom(dpy, "_NET_WM_STATE", 0 );
	if( flags2 & XEHI_SWS_SkipTaskbarOn ){
		// ref: https://cygwin.com/ml/cygwin-xfree/2008-11/msg00388.html
		Atom xaSkipTaskbar = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", 0 );
		// PropModeReplace, PropModePrepend, or PropModeAppend.
		XChangeProperty( dpy, wid, xaNetWmState, XA_ATOM, 32,
					PropModeAppend, (unsigned char*)&xaSkipTaskbar, 1 );
	}
	if( flags2 & XEHI_SWS_SkipPagerOn ){
		Atom xaSkipTaskbar = XInternAtom(dpy, "_NET_WM_STATE_SKIP_PAGER", 0 );
		XChangeProperty( dpy, wid, xaNetWmState, XA_ATOM, 32,
					PropModeAppend, (unsigned char*)&xaSkipTaskbar, 1 );
	}
	if( flags2 & XEHI_SWS_SkipTaskbarOff ){
		//XWithdrawWindow( dpy, wid, 0 );
		XUnmapWindow( dpy, wid );
		XMapWindow( dpy, wid );
	}
	if( flags2 & (XEHI_SWS_AlwaysOnTopOn|XEHI_SWS_AlwaysOnTopOff|
					XEHI_SWS_AlwaysOnBottomOn|XEHI_SWS_AlwaysOnBottomOff|
					XEHI_SWS_SkipPagerOn|XEHI_SWS_MaximizeOn|XEHI_SWS_MaximizeOff) ){
		Atom xaAbove = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", 0 );
		Atom xaBelow = XInternAtom(dpy, "_NET_WM_STATE_BELOW", 0 );
		Atom xaSkpPager = XInternAtom(dpy, "_NET_WM_STATE_SKIP_PAGER", 0 );
		XClientMessageEvent xclient;
		memset( &xclient, 0, sizeof(xclient) );
		xclient.type = ClientMessage;
		xclient.window = wid; // GDK_WINDOW_XID(window);
		xclient.message_type = xaNetWmState;
		xclient.format = 32;
		if( flags2 & XEHI_SWS_AlwaysOnTopOn ){
			xclient.data.l[0] = 1; // add ? _NET_WM_STATE_ADD=1 : _NET_WM_STATE_REMOVE;
			xclient.data.l[1] = xaAbove; //gdk_x11_atom_to_xatom_for_display (display, state1);
			XSendEvent( dpy, //mywin - wrong, not app window, send to root window!
					DefaultRootWindow(dpy), 0,
					SubstructureRedirectMask | SubstructureNotifyMask,
					(XEvent*)&xclient );
		}
		if( flags2 & (XEHI_SWS_MaximizeOn|XEHI_SWS_MaximizeOff) ){
			Atom xaMaxH = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", 0 );
			Atom xaMaxV = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", 0 );
			XClientMessageEvent xclient2 = xclient;
			//_NET_WM_STATE_ADD=1,_NET_WM_STATE_REMOVE=0,_NET_WM_STATE_TOGGLE=2.
			xclient2.data.l[0] = ( flags2 & XEHI_SWS_MaximizeOff ? 0 : 1 );
			xclient2.data.l[1] = xaMaxH; //ok
			xclient2.data.l[2] = xaMaxV; //ok
			xclient2.data.l[3] = 1;
			XSendEvent( dpy, DefaultRootWindow(dpy), 0,
					SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*)&xclient2 );
			// ---------------------------------------
			// NOTE: Works. BUT, only works bcause '_NET_WM_STATE_SKIP_TASKBAR'
			//       is allocated ??
			//       LXPanel 0.6.1, Desktop panel for LXDE project, (C)2011, http://lxde.org/.
			// ---------------------------------------
			Atom xaSt = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", 0 );//0,1
			//printf("atom-name: [%s]\n", xehi_GetAtomName(dpy,xaSt).c_str() );
			xaSt = xaSt;
		}//*/
	/*	if( flags2 & XEHI_SWS_MaximizeOff ){
			Atom xaMaxH = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", 0 );
			Atom xaMaxV = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", 0 );
			XClientMessageEvent xclient2 = xclient;
			xclient2.data.l[0] = 0; //_NET_WM_STATE_ADD=1,_NET_WM_STATE_REMOVE=0,_NET_WM_STATE_TOGGLE=2
			xclient2.data.l[1] = xaMaxH; //ok
			xclient2.data.l[2] = xaMaxV; //ok
			xclient2.data.l[3] = 1;
			XSendEvent( dpy, DefaultRootWindow(dpy), 0,
					SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*)&xclient2 );
			XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", 0 );
		}//*/

		if( flags2 & XEHI_SWS_AlwaysOnTopOff ){
			xclient.data.l[0] = 0;
			xclient.data.l[1] = xaAbove;
			XSendEvent( dpy, DefaultRootWindow(dpy), 0,
					SubstructureRedirectMask | SubstructureNotifyMask,
					(XEvent*)&xclient );
		}
		if( flags2 & XEHI_SWS_AlwaysOnBottomOn ){
			xclient.data.l[0] = 1;
			xclient.data.l[1] = xaBelow;
			XSendEvent( dpy, DefaultRootWindow(dpy), 0,
					SubstructureRedirectMask | SubstructureNotifyMask,
					(XEvent*)&xclient );
		}
		if( flags2 & XEHI_SWS_AlwaysOnBottomOff ){
			xclient.data.l[0] = 0;
			xclient.data.l[1] = xaBelow;
			XSendEvent( dpy, DefaultRootWindow(dpy), 0,
					SubstructureRedirectMask | SubstructureNotifyMask,
					(XEvent*)&xclient );
		}

		if( flags2 & XEHI_SWS_SkipPagerOn ){
			xclient.data.l[0] = 1;
			xclient.data.l[1] = xaSkpPager;
			XSendEvent( dpy, DefaultRootWindow(dpy), 0,
					SubstructureRedirectMask | SubstructureNotifyMask,
					(XEvent*)&xclient );
		}
	}
	return 1;
}
/// Just wraps its Xlib equivalent into C++ function.
std::string xehi_GetAtomName( Display* dpy, Atom atom2 )
{
	std::string str;
	char* szAtom2 = XGetAtomName( dpy, atom2 );
	if( szAtom2 ){
		str = szAtom2;
		XFree( szAtom2 );
	}
	return str;
}
Window xehi_GetParentWindow( Display* dpy, Window wid )
{
	Window rootw = 0, parentw = 0, *childrenw = 0; unsigned int nchi;
	if( !XQueryTree( dpy, wid, &rootw, &parentw, &childrenw, &nchi ) )
		return 0;
	if( childrenw ){
		XFree( childrenw );
	}
	return parentw;
}
/// Gets input focus window.
/// flags4 - flags, fe. \ref XEHI_GIFW_FirstFound.
/// // TODO: 'int* winPidOu' --> .../x11_xlib_programming/11_x11_maximize_window_tests/maximizesolib/entrypoint.cpp
/// //                       --> app_GetInputFocusWindow()
Window xehi_GetInputFocusWindow( Display* dpy, int flags4, int* winPidOu )
{
	Window wFocus = 0, wid; int dmy2;
	XGetInputFocus( dpy, &wFocus, &dmy2 );
	if( flags4 & XEHI_GIFW_FirstFound )
		return wFocus;
	if( wFocus ){
		std::string str;
		for( wid = wFocus; wid; wid = xehi_GetParentWindow(dpy,wid) ){
			str = xehi_GetWindowPropertyAsText( dpy, wid, "WM_CLASS", 0, 5 );
			//printf("WM_CLASS: prop:[%s] %d, t:[%s]\n", str.c_str(), (int)wid, xehi_GetWindowName(dpy,wid).c_str() );
			if( !str.empty() )
				break;
		}
		if(winPidOu){
			*winPidOu = 0;
			std::vector<uint8_t> data2;
			xehi_GetWindowProperty2( dpy, wid,
					XInternAtom( dpy, "_NET_WM_PID", 0 ),
					-1, &data2, 0,0,0,0, 0 );
			if( data2.size() >= 4 ){
				*winPidOu = *((int*) &data2[0] );
			}
		}
		return wid;
	}
	return 0;
}

