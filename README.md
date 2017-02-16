
	XW-Mod-Util (xwmodutil.so)
= = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

	Utility in the form of a dynamically linked, shared library that, 
	once attached to the initializing process,
	can perform runtime actions or add new functionalities.
	Examples include: maximize window, change application icon
	or execute shell command.

----------------------------------
	Build Instructions
----------------------------------

	* Enter project directory, the "makefile" is located there.
	* Use command: "make release" to build the library.
	* On success, new file "xwmodutil.so" is created under "/bin" subdir.
	* Thats it, copy the shared library to your directory or create symlink to it.
	  There is no system wide install step.


-------------------------
	Configuration
------------------------

	Configuration must be done using the environment variable named 
	XWMODUTIL_CFG. All selectors and actions must be specified 
	as colon separated list.
	
	
	SELECTORS
	--------------
		Selectors provide a way on how to search for the target window in the
		initializing process. Just like actions, at least one must be 
		added to the configuration environment variable.
	
		bOnFirstInput=1
			Action is activated by first user input. Key press,
			mouse button press or mouse move.
		
		nSearchWndAfter=ms
			Search for the target window by: (1) waiting specified amout of time,
			then (2) checking if active window is from the current process.
			Search is performed only one time. This is the crude way, if 
			the encountered active window does not belong to the current 
			process, no action is taken. Parameter is the timeout value 
			in milisecsonds (eg. 1000 is 1 second).
		
		nUseMapFromTo=FROM,TO
			Action is performed on every N-th call to the XMapWindow(),
			while N >= FROM and N < TO.
			Eg. use 'nUseMapFromTo=3,4' to perform the actions when
			the process performs it's third call to the XMapWindow().
			XMapWindow() is the X11 funtion from the Xlib. Programs use it
			to make every window they create visible. Using this API funtion, 
			rather than one of create-window ones, seems to be more reliable.


	WINDOW ACTIONS
	------------------
		
		Preferably, keep terminal window open to see what results
		XWMODUTIL prints to the STDOUT to tweak it's configuration.
	
		mx2
			Maximize window.
				
		mi2
			Minimize window.
		
		exec2=c
			Execute a shell command once target window has been selected.
			Thanks to text tags, shell command can receive window identifier
			and do further processing. Also see note#2.
			Tag list: @pid@, @wid@ and @dpy@.
			
		uwp2
			Update window pid property. The "_NET_WM_PID".
		
		swi2=f
			Set window icon.
			Please note that image loading routine is very simple, 
			Though only one format is supported, it is full 32-bit
			per pixel format with alpha channel translucentcy.
			Only PAM images with RGBA pixel format are supported.
			Value must specify valid path to the PAM image.
			Example image size: 64x64.
			Command example, using Ffmpeg, to convert to comaptible format:
			
				$> ffmpeg -i ./a.png -an -f image2 -c:v pam -pix_fmt rgba ./a.pam
			
			(See also note#2).
				
		swg2=s
			Set window geometry (same as '-geometry' switch from Gnome
			environment (format: WxH+X+Y)).
			Eg: "swg2=400x300+64+32"


	On PROCESS init ACTIONS
	----------------------------
		Actions on XWMODUTIL initialization performed regardless 
		if any window has been selected.
	
		pidfile=f
			Create pid-file that holds current process ID and is automatically
			deleted once process exits (see note#2).
				
		szActiveIf=t
			Optional condition that determines whenever XWMODUTIL should
			switch into dormant state. By default XWMODUTIL is in active state.
			A text that is to be matched on argv[0] of the current process.
			Simple text matching only.
				
		bClearPreload
			If set, XWMODUTIL clears itself from the LD_PRELOAD env var.
			Note: simple string matching, name of shared library cannot be changed.
			This action is performed only if not switched into dormant state
			(see 'szActiveIf').

	note#2:
		Percent-decoding where indicated. Encoding detection is by tail characters,
		ie. decoding is done if the last 3 characters have been set to the
		'%HH' sequence, eg. '%20'. The '%00' sequence can be used to make 
		that last character removed from the result.


-------------------------
	Examples
-------------------------
	
	# Setup two environment variables, LD_PRELOAD and XWMODUTIL_CFG.
	# Once XWMODUTIL initializes itself, it changes window icon of
	# the 'leafpad' to "icon_file_64x64.pam".
	export LD_PRELOAD=/path/to/xwmodutil.so
	export XWMODUTIL_CFG="bOnFirstInput:swi2=/path/to/icon_file_64x64.pam"
	leafpad
