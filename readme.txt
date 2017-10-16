
= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
	Arrow Server - HTTP Server to Shell Command Filtering
= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

	Configurable, non-interactive HTTP server that
	executes shell commands in response to network requests.
	Comprehensive filtering of the requests is possible through
	command-line text tags.
	Offers minimum dependencies and statically build executables.
	Linux and Windows builds available.


--------------------------
	Table Of Contents
--------------------------
	- Connecting                [vg3rdm]
	- Build Instructions        [hikyx9]
	- Usage                     [va5sch]
	- Examples                  [pzkwi7]


---------------------
	Connecting                            [vg3rdm]
---------------------
	Use this web browser URL, on the local machine,
	to connect to the server:
		http://127.0.0.1:7007/

	Using Wget, on the local machine, use this command to
	test HTTP connection:
		$> wget "http://127.0.0.1:7007/" -O-

-------------------------------
	Build Instructions                      [hikyx9]
-------------------------------

	Enter directory with the 'makefile' in it,
	use this command:

		$> make release

	Substitute 'release' with: 'release_s', 'release_w32' or
	'dbg' to build specific target.


----------------
	Usage                                    [va5sch]
----------------
	On commandline every parameter must be made out of two arguments,
	regardles if specyfying text, integer or boolean value.
	Boolean values must be set using either decimal 0 or decimal 1.
	Parsing will be undefined if boolean parameter has been assigned
	to some other text like "true" or "NULL". Please see examples below.

	--nPortNr <NUM>
			HTTP server port number to start to listen on for the incomming
			connections.
			default: 7007

	--Cmd <TEXT>
			Shell command to execute on every client HTTP request.
			Function used is "system()" from the libc.
			Available search-replace text tags:
				{c.ipaddr} {c.hostname} {c.idConn} {c.uTotalBytes} {c.rq}
				{s.nPortNr} {s.version} {s.uMaxRqSizeB} {s.nPurged}
				{rq.<varname>}

			Text tag "{rq.<varname>}" inserts actual value of the HTTP request variable
			from the request URL. Replace '<varname>' with the request variable name.
			Fe. if client request was "/file.txt?time=100ms",
			then "{rq.time}" inserts text: "100ms".
			Inserted text is percent escaped. Escaped character-set does not include
			lettters, numbers and a handfull of punctuators (dot, dash, underscore).
			See the source code of the 'HssdCli::fireShellCommandsIfAny()' function
			for complete reference.

	--bShowIdleSw <0|1>, default: 1
	--bShowHttpRqText <0|1>, default: 0
	--bShowCmdOpts <0|1>, default: 0
	--bShowCaption <0|1>, default: 1
	--bShowConnecting <0|1>, default: 0
	--bSendVerAsStatus <0|1>, default: 1
	--bShowWarnings <0|1>, default: 0

	--bUseSigInt - <0|1>
			If set to 0, there will be no clean exit attempt on
			Ctrl-C interrupt signal.
			default: 1

	--uMaxRqSizeB <NUM>
			Maximum request size, in bytes, from the client, including size of
			the headers and the message body.
			default: 65536

	--aSleepPerformance <MS,MS,MS>
			Three values, format:
				<active-sleep-ms>,<threshold-ms>,<idle-sleep-ms>
			Second value, <threshold-ms>, determines the time
			it is needed to elapse before switch into the idle mode.
			First and third values are times in miliseconds the message loop
			waits between network query operations.
			default: "50,2000,500"

	--aWhiteListCIPs <TEXT>
			White list IP addresses as comma separated list.
			Allowed clients that can be processed by the server.
			Leaving it empty causes no IP restrictions.
            Only simple IP-address format is supported.
            Four digit, dot separated. IPv4 only.
            default: EMPTY

	--uPurgeTimeout <MS>
			The time after client gets disconnected,
			Whenever it never sends data, sends incomplete data or
			does not disconnected itself from the server.
			default: 16000 (16 seconds)

	--uHammerInterval <MS>
			The time the client is allowed to make another HTTP request.
			default: 500

	--nMaxConnPerServer <NUM>
			Maximum number of connections, total (per server).
			default: 8

	--nMaxConnPerClient <NUM>
			Maximum number of connections per client, total.
			Clients are distinguished by their IP addresses.
			default: 4

	--nMaxConnPerMsPerServer <NUM/MS>
			Maximum number of connections per miliseconds per server.
			This parameter must be set to two values, separated by the slash character.
			Eg. 10/1000, first value is tne number of connections
			and second is time in miliseconds.
			default: 32/1000

	--nMaxConnPerMsPerClient <NUM/MS>
			Maximum number of connections per miliseconds per client.
			This is equivalent of '--nMaxConnPerMsPerServer' argument but with
			per-client scope.
			Clients are being distinguished by their IP addresses (IPv4).
			default: 16/1000


-------------------
	Examples                            [pzkwi7]
-------------------

	$> arsrv --nPortNr 7007 --aWhiteListCIPs "1.1.1.1, 127.0.0.1" \
			--bShowHttpRqText 1 --bShowConnecting 1 \
			--bSendVerAsStatus 1 --Cmd "echo A: {c.rq}"

