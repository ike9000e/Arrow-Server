#include "entrypoint.h"

static SMSL cfg2;
static bool bSoActivated = 1, bInputEventOnce = 0;
static int nCntMapWnd = 0;

// instantations.
template bool app_LoadPAMRgbaImageSimple( const char*, int, std::vector<uint64_t>&, std::string*, int*, int* );
template bool app_LoadPAMRgbaImageSimple( const char*, int, std::vector<uint32_t>&, std::string*, int*, int* );

/**
	Create thread.
	\code
		// example.
		app_CreateThread( []( void* ){
			return (void*)0;
		}, 0 );
	\endcode
*/
bool app_CreateThread( void*(*calb2)(void*), void* param2 )
{
	pthread_t thread2;
	int iret1 = pthread_create( &thread2, 0, calb2, (void*)param2 );
	if(iret1){
		printf("XWMODUTIL: ERROR: pthread_create() failed, return code %d\n",iret1 );
		return 0;
	}
	return 1;
}
/**
	Gets command line non-standard way.
	Deditated for use in shared libraries (SO) or DLLs,
	argc|argv isn't normally available there.

	\param uPid - optional, zero retrieves commandline of current process.

	\verbatim
		Linux: Shared lib or DLL.
		>> On Linux the pseudo-file /proc/self/cmdline holds the command line
		>> for the process. Each argument is terminated with a 0 byte, and the
		>> final argument is followed by an additional 0 byte.
		ref: http://stackoverflow.com/questions/160292/how-can-i-access-argc-and-argv-in-c-from-a-library-function

		WIN32 WINAPI: GetCommandLine()
		>> then use CommandLineToArgvW() to convert that
		>> pointer to argv[] format. There is only a wide (Unicode) version
		>> available, though.
	\endverbatim
*/
bool app_GetCommandLineSOSpec( std::vector<std::string>& outp, uint32_t uPid )
{
	char bfrFn[128] = "/proc/self/cmdline";
	if( uPid )
		sprintf( bfrFn, "/proc/%u/cmdline", uPid );
	FILE* fp2 = fopen( bfrFn, "rb" );
	if(fp2){
		rewind(fp2);
		char bfr[128], bfr2[2] = {0,0,};
		size_t num, cnt = 0;
		outp.push_back("");
		for(;;){
			num = fread( bfr, 1, sizeof(bfr), fp2 );
			for( size_t i=0; i<num; i++ ){
				if( bfr[i] ){
					bfr2[0] = (char) bfr[i];
					outp.back().append( bfr2 );
				}else{
					if( !outp.back().empty() )
						outp.push_back("");
				}
			}
			cnt += num;
			if( feof(fp2) || ferror(fp2) )
				break;
			if( cnt >= 65536 ) // sanity check.
				break;
		}
		if( !outp.empty() && outp.back().empty() ) // if last is empty, remove it.
			outp.resize( outp.size() - 1 );
		fclose(fp2);
		return !outp.empty();
	}
	return 0;
}
bool app_SetWindowPropertyAsCardinal( Display* dpy, Window wid, const char* property2, unsigned long val )
{
	// shell xprop examples:
	// * [_NET_WM_PID(CARDINAL) = 8775]
	// * [_OB_APP_CLASS(UTF8_STRING) = "XTerm"]
	// * [WM_LOCALE_NAME(STRING) = "en_US.UTF-8"]
	int rs2 = 0;
	rs2 = XChangeProperty( dpy, wid,
						XInternAtom( dpy, property2, 0 ),
						XA_CARDINAL, 32, PropModeReplace,
						(unsigned char*)&val, 1 );
	return !rs2;
}

// more or less, equivalent of: 'xehi_SetWindowNetWmStates(,,XEHI_SWS_MaximizeOn)'
bool app_MaximizeWindow( Display* dpy, Window wid )
{
	if( !dpy ){
		if( !(dpy = XOpenDisplay( 0 )) )
			return 0;
	}
	if(!wid){
		wid = xehi_GetInputFocusWindow( dpy, 0, 0 );
		//printf("XWMODUTIL: maximize-wid: %d.\n", (int)wid );
	}else{
		wid = xehi_GetAppTitleWindow( dpy, wid, XEHI_GATW_ByWmClass );
		printf("XWMODUTIL: To top parent, wid: %d.\n", (int)wid );
	}
	if( wid ){
		printf("XWMODUTIL: Maximizing %d ...\n", (int)wid );
		Atom xaNetWmState = XInternAtom(dpy, "_NET_WM_STATE", 0 );
		Atom xaMaxH = 0, xaMaxV = 0;
		xaMaxH = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", 0 );
		xaMaxV = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", 0 );
		XClientMessageEvent xclient;
		memset( &xclient, 0, sizeof(xclient) );
		xclient.type = ClientMessage;
		xclient.window = wid; // GDK_WINDOW_XID(window);
		xclient.message_type = xaNetWmState;
		xclient.format = 32;
		//_NET_WM_STATE_ADD=1,_NET_WM_STATE_REMOVE=0,_NET_WM_STATE_TOGGLE=2.
		xclient.data.l[0] = 1;
		xclient.data.l[1] = xaMaxH; //ok
		xclient.data.l[2] = xaMaxV; //ok
		xclient.data.l[3] = 1;//1
		XSendEvent( dpy, DefaultRootWindow(dpy), 0,
				SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*)&xclient );
		// ---------------------------------------
		// NOTE: Works. BUT, only works bcause '_NET_WM_STATE_SKIP_TASKBAR'
		//       is allocated ??
		//       LXPanel 0.6.1, Desktop panel for LXDE project, (C)2011, http://lxde.org/.
		// ---------------------------------------
		Atom xaSt = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", 0 );//0,1
		//printf("XWMODUTIL: atom-name: [%s]\n", xehi_GetAtomName(dpy,xaSt).c_str() );
		xaSt = xaSt;
		return 1;
	}
	return 0;
}
std::string app_StrVal( const int64_t inp, bool bFirst, uint64_t inp2 )
{
	char bfr[64];
	if(bFirst)
		sprintf( bfr, "%ld", inp );
	else
		sprintf( bfr, "%lu", inp2 );
	return bfr;
}
std::vector<std::string> app_StrExplode( const char* inp, const char* glue2, int nLimit )
{
	std::vector<std::string> z; const char* ptr; const int len = strlen(glue2);
	for(; nLimit && (ptr = strstr( inp, glue2 )); nLimit-- ){
		z.push_back( std::string( inp, ptr-inp ) );
		inp = ptr + len;
	}
	if(*inp)
		z.push_back( inp );
	return z;
}

/// S-R.
/// \param srch - must has last element set to 0.
std::string app_SearchReplace( const char* inp, const char*const* srch,
				const std::vector<std::string>& repl2 )
{
	std::string z; const char* sz2; std::pair<const char*,int> rs2(0,0);
	for(;; rs2.first = 0 ){
		for( int k=0; srch[k]; k++ ){
			if( (sz2 = strstr(inp,srch[k])) && ( !rs2.first || sz2 < rs2.first ) )
				rs2 = std::pair<const char*,int>( sz2, k );
		}
		if( rs2.first ){
			z.append( inp, rs2.first - inp );
			z.append( repl2[rs2.second] );
			inp = rs2.first + strlen( srch[rs2.second] );
		}else
			break;
	}
	z.append( inp );
	return z;
}
void app_ProceedToAction( const SPTA& pta )
{
	{
		if( cfg2.swg2.W > 0 && cfg2.swg2.X >= 0 ){ // W,H,X,Y --> WxH+X+Y
			XMoveResizeWindow( pta.dpy, pta.wid,
					cfg2.swg2.X, cfg2.swg2.Y, cfg2.swg2.W, cfg2.swg2.H );
		}else if( cfg2.swg2.W > 0 ){
			XResizeWindow( pta.dpy, pta.wid, cfg2.swg2.W, cfg2.swg2.H );
		}else if( cfg2.swg2.X >= 0 ){
			XMoveWindow( pta.dpy, pta.wid, cfg2.swg2.X, cfg2.swg2.Y );
		}
	}
	if( cfg2.mx2 ){
		app_MaximizeWindow( pta.dpy, pta.wid );
	}
	if( cfg2.mi2 ){
		int screen2;
		// Get screen number. Taken from 'xdotool' -> xdo.c -> xdo_minimize_window()
		XWindowAttributes attr;
		XGetWindowAttributes( pta.dpy, pta.wid, &attr);
		screen2 = XScreenNumberOfScreen( attr.screen );
		printf("XWMODUTIL: minimize window, wid:%d, screen:%d\n", (int)pta.wid, screen2 );
		XIconifyWindow( pta.dpy, pta.wid, screen2 );
	}
	if( cfg2.bUpdWndPid ){
		printf("XWMODUTIL: update window pid, wid:%d\n", (int)pta.wid );
		bool rs2 = app_SetWindowPropertyAsCardinal( pta.dpy, pta.wid, "_NET_WM_PID", pta.wid );
		//printf("rs2: %d\n", (int)rs2 );
		rs2=0;
	}
	if( !cfg2.exec2->empty() ){
		std::string cmd2 = cfg2.exec2->c_str(), cmd3; const char* sz, *sz2;
		const char* srch[] = { "@pid@", "@wid@", "@dpy@", 0, };
		std::vector<std::string> repl2;
		repl2.push_back( app_StrVal( getpid() ) );
		repl2.push_back( app_StrVal( (int64_t)pta.wid ) );
		repl2.push_back( app_StrVal( 0, 0, (uint64_t)pta.dpy ) );
		cmd2 = app_SearchReplace( cmd2.c_str(), srch, repl2 );
		printf("XWMODUTIL: cmd: [%s]\n", cmd2.c_str() );
		int x = system( cmd2.c_str() );
		x++; //silencing.
	}
	if( !cfg2.swi2->empty() ){
        std::vector<unsigned long> data3;
        bool rs3 = app_LoadPAMRgbaImageSimple<unsigned long>( cfg2.swi2->c_str(),
				HEF_PAMSL_PrependWH|HEF_PAMSL_FormatARGB, data3, 0, 0, 0 );
        //printf("rs3: %d, size:%d\n", (int)rs3, (int)data3.size() );
        printf("XWMODUTIL: Loading PAM image. OK:%d\n", (int)rs3 );
        if(rs3)
			app_SetWindowIcon( pta.dpy, pta.wid, &data3[0], (int)data3.size() );
	}
}
/*
int main( int argc, const char*const* argv )
{
	if(bMainOnce++)
		return 1;
	printf("XWMODUTIL: pid: %d\n", getpid() );
	if(argc)
		usleep(2000000);
	app_MaximizeWindow_( 0, 0 );

	printf("XWMODUTIL: exitting...\n");
	return 0;
}//*/
void SMSL::init2()
{
	nInitedB = nInitedA = 0;
	bClearPreload = bOnFirstInput = mx2 = mi2 = bUpdWndPid = 0;
	nUseMapFromTo.first = nUseMapFromTo.second = 0;
	swg2.W = swg2.H = 0;
	swg2.X = swg2.Y = -1;
	szActiveIf = new std::string("");
	exec2 = new std::string("");
	swi2 = new std::string("");
	szPidFile = new std::string("");
	nSearchWndAfter = -1;
}
void SMSL::init3()
{
	nInitedA = 200101;
	nInitedB = ~nInitedA;
}
bool SMSL::isIntedAB()const
{
	return nInitedB == ~nInitedA;
}
/// Simple percent decode string. Condition is that last 3 chars must be "%HH".
std::string app_PercentDecodeIf( const char* inp, bool* bDecoded )
{
	int len; const char* taill; char chrs[16], *dmy0; int val;
	if( (len=strlen(inp)) >= 3 ){
		taill = &inp[len-3];
		if( taill[0] == '%' && isxdigit( taill[1] ) && isxdigit( taill[2] ) ){
			std::string z;
			for( int i=0; i<len; i++ ){
				if( inp[i] == '%' && i+2<len && isxdigit( inp[i+1] ) && isxdigit( inp[i+2] ) ){
					sprintf(chrs,"%c%c", inp[i+1], inp[i+2] );
					val = (int)strtoul( chrs, &dmy0, 16 );
					sprintf(chrs,"%c", (char)val );
					z += chrs;
					i+= 2;
				}else{
					z += inp[i];
				}
			}
			if(bDecoded) *bDecoded = 1;
			return z;
		}
	}
	return inp;
}
std::string SMSL::toStr( const SMSL& inp, const char* tbs )
{
	std::string z; char bfr[512]; const int n = sizeof(bfr);
	snprintf(bfr,n,"%s""bClearPreload=%d\n", tbs, inp.bClearPreload );
	z += bfr;
	snprintf(bfr,n,"%s""nUseMapFromTo=%d,%d\n", tbs, inp.nUseMapFromTo.first, inp.nUseMapFromTo.second );
	z += bfr;
	snprintf(bfr,n,"%s""bOnFirstInput=%d\n", tbs, inp.bOnFirstInput );
	z += bfr;
	snprintf(bfr,n,"%s""szActiveIf=%s\n", tbs, inp.szActiveIf->c_str() );
	z += bfr;
	snprintf(bfr,n,"%s""mx2=%d\n", tbs, (int)inp.mx2 );
	z += bfr;
	snprintf(bfr,n,"%s""mi2=%d\n", tbs, (int)inp.mi2 );
	z += bfr;
	snprintf(bfr,n,"%s""bUpdWndPid=%d\n", tbs, (int)inp.bUpdWndPid );
	z += bfr;
	if( strstr( inp.exec2->c_str(), " " ) ){
		snprintf(bfr,n,"%s""exec2: [%s]\n", tbs, inp.exec2->c_str() );
		z += bfr;
	}else{
		snprintf(bfr,n,"%s""exec2=%s\n", tbs, inp.exec2->c_str() );
		z += bfr;
	}
	snprintf(bfr,n,"%s""nSearchWndAfter=%s\n", tbs, ( inp.nSearchWndAfter != -1 ? app_StrVal(inp.nSearchWndAfter).c_str() : "" ) );
	z += bfr;
	snprintf(bfr,n,"%s""swi2=%s\n", tbs, inp.swi2->c_str() );
	z += bfr;
	snprintf(bfr,n,"%s""pidfile=%s\n", tbs, inp.szPidFile->c_str() );
	z += bfr;
	snprintf(bfr,n,"%s""swg2=%d""x%d+%s+%s\n", tbs,
			inp.swg2.W, inp.swg2.H,
			( inp.swg2.X >= 0 ? app_StrVal( inp.swg2.X ).c_str() : "" ),
			( inp.swg2.Y >= 0 ? app_StrVal( inp.swg2.Y ).c_str() : "" ) );
	z += bfr;
	//
	return z;
}

void app_ReadCfgFromEnv()
{
	const char* envv = getenv("XWMODUTIL_CFG");//, *sz, *endd;
	if( envv && *envv ){
		std::string nme, val; const char* sz2;
		std::vector<std::string> envva = app_StrExplode( envv, ":", -1 ), arr;
		for( size_t i=0; i < envva.size(); i++ ){
			arr = app_StrExplode( envva[i].c_str(), "=", 1 );
			nme = ( !arr.empty() ?    arr[0].c_str() : "" );
			val = ( arr.size() >= 2 ? arr[1].c_str() : "" );
			if(0){
			}else if( nme == "bClearPreload" ){
				cfg2.bClearPreload = !!atoi( !val.empty() ? val.c_str() : "1" );
			}else if( nme == "nUseMapFromTo" ){
				sscanf(val.c_str(),"%d,%d", &cfg2.nUseMapFromTo.first, &cfg2.nUseMapFromTo.second );
			}else if( nme == "bOnFirstInput" ){
				cfg2.bOnFirstInput = !!atoi( !val.empty() ? val.c_str() : "1" );
			}else if( nme == "szActiveIf" ){
				*cfg2.szActiveIf = val;
			}else if( nme == "mx2" ){
				cfg2.mx2 = !!atoi( !val.empty() ? val.c_str() : "1" );
			}else if( nme == "mi2" ){
				cfg2.mi2 = !!atoi( !val.empty() ? val.c_str() : "1" );
			}else if( nme == "uwp2" ){
				cfg2.bUpdWndPid = !!atoi( !val.empty() ? val.c_str() : "1" );
			}else if( nme == "exec2" ){
				*cfg2.exec2 = app_PercentDecodeIf( val.c_str() );
			}else if( nme == "nSearchWndAfter" ){
				sscanf(val.c_str(),"%d", &cfg2.nSearchWndAfter );
			}else if( nme == "swi2" ){
				*cfg2.swi2 = app_PercentDecodeIf( val.c_str() );
			}else if( nme == "pidfile" ){
				*cfg2.szPidFile = app_PercentDecodeIf( val.c_str() );
			}else if( nme == "swg2" ){
				if( (sz2 = strstr(val.c_str(),"x")) ){
					cfg2.swg2.H = atoi( sz2+1 );
					cfg2.swg2.W = atoi( val.c_str() );
				}
				if( (sz2 = strstr(val.c_str(),"+")) ){
					cfg2.swg2.X = atoi( ++sz2 );
					cfg2.swg2.Y = atoi( (sz2 = strstr(sz2,"+")) ? sz2+1 : "" );
				}
			}
		}
	}
	printf("XWMODUTIL(%d): Config:\n%s", getpid(), cfg2.toStr(cfg2,"\t").c_str() );
}
/*
//XCreateWindow, XCreateSimpleWindow
typedef Window (*XCreateWindow_t)( Display* display, Window parent, int x, int y,
			unsigned int width, unsigned int height,
			unsigned int border_width, int depth,
			unsigned int classx, Visual* visual, unsigned long valuemask,
			XSetWindowAttributes* attributes );
extern "C" Window XCreateWindow( Display* display, Window parent, int x, int y,
			unsigned int width, unsigned int height,
			unsigned int border_width, int depth,
			unsigned int classx, Visual* visual, unsigned long valuemask,
			XSetWindowAttributes* attributes )
{
	XCreateWindow_t fnc2 = (XCreateWindow_t) dlsym(RTLD_NEXT, "XCreateWindow");
	Window retv = fnc2( display, parent, x,y,width,height, border_width,
					depth, classx, visual, valuemask, attributes );
	printf("XWMODUTIL: XCreateWindow_t() %d\n",  );
	if( bSoActivated && ){
	}
	return retv;
}//*/
typedef int (*XNextEvent_t)( Display*, XEvent* );
extern "C" int XNextEvent( Display* display, XEvent* evt2 )
{
	XEvent& evt = *evt2;
	XNextEvent_t fnc4 = (XNextEvent_t) dlsym(RTLD_NEXT, "XNextEvent");
	int retv = fnc4( display, &evt );
	if( bSoActivated && cfg2.bOnFirstInput && !bInputEventOnce ){
		if( evt.type == KeyPress || evt.type == MotionNotify || evt.type == ButtonPress ){
			bInputEventOnce = 1;
			const XAnyEvent& evt3 = (XAnyEvent&)evt;
			printf("XWMODUTIL: XNextEvent input-once, wid:%d\n", (int)evt3.window );
			//app_MaximizeWindow_( evt3.display, evt3.window );
			app_ProceedToAction( SPTA( evt3.display, evt3.window ) );
		}
	}
	return retv;
}
typedef void(*XMapWindow_t)( Display*, Window );
extern "C" int XMapWindow( Display* dpy, Window wid )
{
	static XMapWindow_t fnc3 = 0;
	if(!fnc3)
		fnc3 = (XMapWindow_t) dlsym(RTLD_NEXT, "XMapWindow");
	//printf("XWMODUTIL(%d): XMapWindow, wid:%d, i:%d, (map-if:%d,%d), inited-ab:%d\n",
	//				getpid(), (int)wid, nCntMapWnd,
	//				cfg2.nUseMapFromTo.first, cfg2.nUseMapFromTo.second, (int)cfg2.isIntedAB() );
	fnc3(dpy,wid);
	if( bSoActivated && nCntMapWnd >= cfg2.nUseMapFromTo.first && nCntMapWnd < cfg2.nUseMapFromTo.second ){
		printf("XWMODUTIL: XMapWindow, wid:%d, match:%d\n", (int)wid, nCntMapWnd );
		app_ProceedToAction( SPTA( dpy, wid ) );
	}
	nCntMapWnd++;
	return 0;
}

void app_SoLibInit()
{
	std::string arg0; const char* sz;
	cfg2.init2();//isIntedAB()
	app_ReadCfgFromEnv();
	if( !cfg2.szActiveIf->empty() ){
		std::vector<std::string> argv2;
		app_GetCommandLineSOSpec( argv2, 0 );
		arg0 = ( !argv2.empty() ? argv2[0] : "" );
		bSoActivated = !!strstr( arg0.c_str(), cfg2.szActiveIf->c_str() );
		arg0 = ( (sz = strrchr(arg0.c_str(),'/')) ? sz+1 : arg0.c_str() );
	}
	printf("XWMODUTIL(%d): Active-on-init:%d, pid:%d [%s]\n",
				getpid(),
				(int)bSoActivated, (int)getpid(), arg0.c_str() );
	if( bSoActivated ){
		if( cfg2.bClearPreload ){
			const char* envv = getenv("LD_PRELOAD");
			if( envv && *envv ){
				if( (sz=strstr(envv,"xwmodutil.so")) ){
					printf("XWMODUTIL: Removing itself from LD_PRELOAD.\n");
					std::string str = "LD_PRELOAD=";
					str.append( envv, sz-envv );
					str.append( "_" );
					str.append( sz+1 );
					putenv( (char*)str.c_str() );
				}
			}
		}
		if( cfg2.nSearchWndAfter != -1 ){
			app_CreateThread( [&](void*){
				usleep( cfg2.nSearchWndAfter * 1000 );
				int wpid = 0;
				Window wid = xehi_GetInputFocusWindow( XOpenDisplay(0), 0, &wpid );
				printf("XWMODUTIL: nSearchWndAfter, wpid:%d\n", wpid );
				if( wpid == getpid() ){
					SPTA pta( XOpenDisplay(0), wid );
					app_ProceedToAction( pta );
				}else{
					printf("XWMODUTIL: focus window pid (%d) different than own, skipping.\n", wpid );
				}
				return (void*)0;
			}, 0 );
		}
		if( !cfg2.szPidFile->empty() ){
			char bfr[64];
			sprintf(bfr,"%d",getpid());
			FILE* fp3 = fopen(cfg2.szPidFile->c_str(),"wb");
			if(fp3){
				fwrite( bfr, 1, strlen(bfr), fp3 );
				fclose(fp3);
			}
		}
	}
	cfg2.init3();//isIntedAB()
}

void app_SoLibDeinit()
{
	if( bSoActivated && !cfg2.szPidFile->empty() ){
		unlink( cfg2.szPidFile->c_str() );
	}
}

/**
	Sets Xlib/X11 window icon.
	\param pxs3 - The pixels data. First two elements must be width and height.
	\param nNumItems - number of items in the 'pxs3' array. ie. number of
	_                  pixels plus 2 (note: data prepended with W and H).
	Once set, data is no longer needed to remain valid in the application, it seems.
	\verbatim
		Note: pixel format for Xlib XChangeProperty(,"_NET_WM_ICON",) call is 0xAARRGGBB.
		-     (or on 64-bit sustem: 0x00000000AARRGGBB (note: 'unsigned long' is 64-bit there)).
	\endverbatim
*/
bool app_SetWindowIcon( Display* dpy, Window wnd, const unsigned long* pxs3, int nNumItems )
{
	Atom property2 = XInternAtom( dpy, "_NET_WM_ICON", 0 );
	if(!property2){
		//xwim_printf( EV_Vrbst_1,"ERROR: !property2\n");
		return 0;
	}
	// pixel format for XChangeProperty(,"_NET_WM_ICON",) call: 0xAARRGGBB.
	// (on 64-bit sustem: 0x00000000AARRGGBB (note: unsigned long is 64-bit)).
	int res = XChangeProperty( dpy, wnd, property2, XA_CARDINAL, 32,
				PropModeReplace, (const unsigned char*)pxs3, nNumItems );
	if(!res){
		//xwim_printf( EV_Vrbst_1,"ERROR: XChangeProperty() failed.\n");
		return 0;
	}
	res = XFlush( dpy );
	if( !res ){
		//xwim_printf( EV_Vrbst_1,"ERROR: XFlush() failed.\n");
		return 0;
	}
	return 1;
}
/**
	Loads PAM image into output array.
	Simple PAM format loader that fails if format is not an RGBA (32 bits per pixel).
	The 'TUPLTYPE' entry in the PAM header must be 'RGB_ALPHA'.
	Format details: [https://en.wikipedia.org/wiki/Netpbm#PAM_graphics_format].
	\param flags2 - flags, fe. \ref HEF_PAMSL_FormatRGBA.
	\param T - Template parameter. Should be at least 4 bytes in size.
	_          For icons on X11 environment set 'T' to 'unsigned long'.
	\verbatim
		Note: pixel format for Xlib XChangeProperty(,"_NET_WM_ICON",) call is 0xAARRGGBB.
		_     (or on 64-bit sustem: 0x00000000AARRGGBB (note: 'unsigned long' is 64-bit there)).
	\endverbatim
	\code
		// X11 window icon assigning.
        std::vector<unsigned long> data3;
        bool rs3 = app_LoadPAMRgbaImageSimple( "./a.pam",
				HEF_PAMSL_PrependWH|HEF_PAMSL_FormatARGB, data3, .... );
        if(rs3)
			app_SetWindowIcon( dpy, wid, data3, .... );
	\endcode
	\verbatim
		Use this Ffmpeg command to convert any image into format compatible
		with this function:
		_   $> ffmpeg -i ./a.png -an -f image2 -c:v pam -pix_fmt rgba ./a.pam
	\endverbatim
	\verbatim
		PAM graphics format, header and 1x1 image hex-dump exaple.
		------
			P7
			WIDTH 1
			HEIGHT 1
			DEPTH 4
			MAXVAL 255
			TUPLTYPE RGB_ALPHA
			ENDHDR
		------
			"1x1_p7_red_dot_rgba.pam"
			000000   50 37 0A 57 49 44 54 48 20 31 0A 48 45 49 47 48   P7.WIDTH 1.HEIGH
			000016   54 20 31 0A 44 45 50 54 48 20 34 0A 4D 41 58 56   T 1.DEPTH 4.MAXV
			000032   41 4C 20 32 35 35 0A 54 55 50 4C 54 59 50 45 20   AL 255.TUPLTYPE
			000048   52 47 42 5F 41 4C 50 48 41 0A 45 4E 44 48 44 52   RGB_ALPHA.ENDHDR
			000064   0A FF 00 00 FF                                    .....
			_           ^  ^  ^  ^
			_           RR GG BB AA
		------
	\endverbatim

	\sa app_SetWindowIcon()
*/
template<class T>//uint64_t
bool app_LoadPAMRgbaImageSimple( const char* szPAMFileName, int flags2,
				std::vector<T>& outp, std::string* err2,
				int* wdtOut, int* hgtOut )
{
	std::string err3, &err = ( err2 ? *err2 : err3 ); char* sz2; T pixel2;
	size_t readd;
	outp.clear();
	FILE* fp2 = fopen(szPAMFileName,"rb");
	bool bSuccess = 0; char bfr[1024] = "";
	do{
		if(!fp2){
			err = "File open failed.";
			break;
		}
		fseek( fp2, 0, SEEK_SET );
		bfr[ fread( bfr, 1, sizeof(bfr)-1, fp2 ) ] = 0;
		if( !(sz2 = (char*)strstr( bfr, "ENDHDR\n" )) ){
			err = "No 'ENDHDR' entry found.";
			break;
		}
		fseek( fp2, (sz2-bfr) + strlen("ENDHDR\n"), SEEK_SET );
		bfr[ sz2-bfr ] = 0;
		// analyze the header.
		if( !(sz2=strstr(bfr,"TUPLTYPE RGB_ALPHA")) ){
			err = "No 'TUPLTYPE RGB_ALPHA' entry found (not a 32 bit RGBA PAM image?).";
			break;
		}
		const int wdt = atoi( (sz2=strstr(bfr,"WIDTH" )) ? (sz2 + strlen("WIDTH" ) + 1) : "");
		const int hgt = atoi( (sz2=strstr(bfr,"HEIGHT")) ? (sz2 + strlen("HEIGHT") + 1) : "");
		if( wdt <= 0 || hgt <= 0 ){
			err = "Invalid image width and/or heaight read.";
			break;
		}
		if(wdtOut) *wdtOut = wdt;
		if(hgtOut) *hgtOut = hgt;
		const int nNumPixels = wdt * hgt, nNumBytes = nNumPixels * 4;
		if( flags2 & HEF_PAMSL_PrependWH ){
			outp.push_back( wdt );
			outp.push_back( hgt );
		}
		int cnt;
		for( cnt = 0; ; ){ // read the pixels.
			readd = fread( bfr, 1, sizeof(bfr), fp2 );
			if( readd % 4 )
				break;
			cnt += readd;
			if( cnt > nNumBytes )
				readd -= ( cnt - nNumBytes );
			// Note:  in PAM, per pixel 4 byte format is: [RR, GG, BB, AA]
			//                                             ^|  ^|  ^|  ^|
			//                                             [0] [1] [2] [3]
			for( int k=0; k<readd; k += 4 ){ // for each pixel in the buffer.
				if( flags2 & HEF_PAMSL_FormatARGB ){
					// pixel format: 0xAARRGGBB (32-bit) and 0x00000000AARRGGBB (64-bit).
					pixel2 =  ( ((T)(bfr[k+0] & 0xFF)) << 16 ); //set r
					pixel2 |= ( ((T)(bfr[k+1] & 0xFF)) << 8 );  //set g
					pixel2 |= ( ((T)(bfr[k+2] & 0xFF)) << 0 );  //set b
					pixel2 |= ( ((T)(bfr[k+3] & 0xFF)) << 24 ); //set a
				}else if( flags2 & HEF_PAMSL_FormatBGRA ){
					pixel2 =  ( ((T)(bfr[k+0] & 0xFF)) << 8 );  //set r
					pixel2 |= ( ((T)(bfr[k+1] & 0xFF)) << 16 ); //set g
					pixel2 |= ( ((T)(bfr[k+2] & 0xFF)) << 24 ); //set b
					pixel2 |= ( ((T)(bfr[k+3] & 0xFF)) << 0 );  //set a
				}else if( flags2 & HEF_PAMSL_FormatABGR ){
					pixel2 =  ( ((T)(bfr[k+0] & 0xFF)) << 0 );  //set r
					pixel2 |= ( ((T)(bfr[k+1] & 0xFF)) << 8 );  //set g
					pixel2 |= ( ((T)(bfr[k+2] & 0xFF)) << 16 ); //set b
					pixel2 |= ( ((T)(bfr[k+3] & 0xFF)) << 24 ); //set a
				}else{ //assume (flags2 & HEF_PAMSL_FormatRGBA).
					pixel2 =  ( ((T)(bfr[k+0] & 0xFF)) << 24 ); //set r
					pixel2 |= ( ((T)(bfr[k+1] & 0xFF)) << 16 ); //set g
					pixel2 |= ( ((T)(bfr[k+2] & 0xFF)) << 8 );  //set b
					pixel2 |= ( ((T)(bfr[k+3] & 0xFF)) << 0 );  //set a
				}
				outp.push_back( pixel2 );
			}
			if( cnt >= nNumBytes )
				break;
			if( feof(fp2) || ferror(fp2) )
				break;
		}
		bSuccess = ( cnt >= nNumBytes );
		if(!bSuccess)
			err = "Failed to read all pixels.";
	}while(0);
	if(fp2)
		fclose(fp2);
	return bSuccess;
}
