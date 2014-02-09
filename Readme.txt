MERVBot Release-candidate (Build 46) Documentation


----=Quick Setup=----

NOTE: Do these steps in order.

Extract all files in archive to an empty folder.

Double-click Spawns.txt
Read the directions.
Delete the example spawns at the very top.
Add your own spawn(s) in the proper format.

Double-click Operators.txt
Read the directions.
Delete the example operators at the very top.
Add yourself in the proper format.

http://stats.trenchwars.org/

Find the zone you want to have the bot enter and write down its "IP Address" and "Port."
Your zone must be listed on one of the main directory servers for it to be listed on that
website.  If it is not listed, you may also find the "IP Address" and "Port" numbers in a
SubSpace favorite.lst file or a Continuum zone.dat file  (Both may be viewed in Notepad)
Or in Continuum, right-click the zone to see it's address.

Double-click MERVBot.INI
Where it says..
	[Login]
	Zone=127.0.0.1:2000
..put the zone's IP Address:Port.

If the zone is Continuum-only, add the bot to VIP.TXT

Double-click MERVBot.EXE

Read the F.A.Q. below if the bot doesn't connect after a minute.
Take this time to read the documentation more thoroughly and to
familiarize yourself with the operator and command systems.

Type /!login <your operator password> to gain access to the bot,
if you didn't set a password for your name at the Operators.txt
step, you will be logged in automatically.

When you are done using the bot, press a key to close it.
DO NOT close the window to close the bot.

Thank you for testing MERVBot!


----=F.A.Q.=----

Q. What is a MERVBot?
A. MERVBot is an open-sourced C++ server-side bot.  Its name comes
   from the U.S. arsenal's nuclear-warhead-equipped MIRV missiles
   because each missile is equipped with multiple warheads.
   Similarly, MERV destroys countries.  Err, it also has multiple
   features to better serve all zones instead of requiring new bots
   to be made for each individual zone.

Q. A plugin is listed in !plugins but it doesn't do anything.  What's up?
A. If the terminal window says there's a version mismatch, download a new version
   of either this bot core or the plugin from my website.  Read "how to upgrade.txt"

Q. How do I specify more than one plugin in spawns.txt or for the !load command?
A. Separate the plugin names with commas: eliza, ctf, flag

Q. I'm getting a MAJOR plugin version mismatch.  How do I fix it?
A. Download the latest version of the plugin and/or the bot.

Q. I'm getting a MINOR plugin version mismatch.  How do I fix it?
A. This is not a problem.  If you download the plugin again, it might have been updated.

Q. WARNING: Super Moderator+ access is required to remain connected in this zone.
   WARNING: Subspace.BIN checksum mismatch!
A. Exactly what it says.  The bot needs to be SMod, SysOp or in vip.txt to remain connected.

Q. Why does the bot get disconnected constantly?
A. 1) If your zone is Continuum-only, it must have Super Moderator/SysOp staff powers.
   2) If your zone is Continuum-only in the SERVER.INI, bots and SubSpace cannot enter.
   One solution to scenario #2 is to add the bot's name to vip.txt

Q. Why do I get "Winsock" problems trying to connect the bot from Windows 95?
A. Try this link for a Winsock update:
   http://www.microsoft.com/windows95/downloads/contents/wuadmintools/s_wunetworkingtools/w95sockets2/

Q. Why doesn't the bot connect?
A. 1) Make sure you have edited MERVBot.INI and have chosen an online zone.
   2) If you are behind a firewall/router, use "SubSpace Proxy 6" from my website.
   3) Ask me about it.

Q. I changed or added an operator while s/he was in the arena and his/her powers haven't changed.
A. Ask him/her to reenter the arena or use !login <password>. You may also change the bot's arena

Q. I deleted an operator while s/he was in the arena and his/her powers haven't been removed.
A. Ask him/her to reenter the arena. You may also change bot's arena

Q. Why doesn't the bot respond to remote private commands from another arena?
A. Change [Misc]RemoteInterpreter=1 in MERVBot.INI, to allow remote commands.
   WARNING: Remote commands may be spoofed by hackers in esoteric circumstances.

Q. How do I create different kinds of bots?
A. 1) Edit Spawns.txt to include the name of the bot DLL file you wish to import.
   2) Type /!import <DLL filename>
   3) Type /!spawn -i=<DLL filename> [other options] <bot name>

Q. Why do I have to constantly type !login !?!?
A. There is no way for the bot to know if it is still you when you change arenas.
   Thus, you must !login whenever you enter a different arena.  You may disable
   the !login password by logging in and typing !setlogin.  Setting a zero-length
   password circumvents the !login system, but means that your account is disabled
   if/when the zone's billing server goes offline (scores are unavailable, etc.)

Q. Why does the bot only respond -once- when I send a public chat command -several- times?
A. This is a limitation imposed by the SubSpace server software.  Interleave with different messages.

Q. What level staff access (mod/smod/sysop) does the bot need?
A. Depends on the application.  Most features will work with
   moderator access, but supermoderator access is necessary if
   you want it to grant prizes.  Being SysOp allows the bot to
   respond faster to commands, but is not necessary unless you
   notice a tremendous lag in response time among other things.

Q. Why isn't MERVBot listening to all the kill messages?
A. Zone settings might need to be corrected.  Type as Arena Owner or Zone SysOp:
   ?set Routing:DeathDistance:16384
   Or download SubGame 11h or later from http://www.shanky.com/server/

Q. Why isn't a MERVBot feature working?
   1) SMods/Mods cannot *spec or *setship SysOps/SMods.
   2) Make sure the bot's name is in the appropriate staff .TXT file for the zone.
      moderate.txt - moderator powers
      smod.txt     - supermoderator powers
      sysop.txt    - sysop powers
   3) Double-check the Spawns.txt file to make sure you didn't make a typo.

Q. Players get kicked for "too many ship-type changes."  How do i fix this?
A. Zone settings must be corrected.  Type as Arena Owner or Zone SysOp:
   ?set Security:MaxShipTypeSwitchCount:32767

Q. How do I make the bot go to a certain public arena?
A. Use command !go <public arena #>.  Ex: !go 3
   Only SysOp's can create public arenas on demand.

Q. Will the bot be safe if the billing server goes down?
A. Yes.  Operators without !login passwords cannot use commands until the billing
   server comes back online.  Remote private messages are ignored completely.

Q. Does MERVBot drain system resources?
A. Bandwidth: 40% average SubSpace session.  ~80 BPS while idling (according to *info)
   Memory: 1MB for one spawn.  For comparison, SubSpace takes 14MB and Continuum takes 6MB.
   CPU: <1% on a PIII 866mhz machine.

Q. What's with the MERVBot_debug.exe?
A. This is a developers tool that removes the runtime exception checking from the core, so
   crashes will fall out into the developers' debugger progam.

Q. I need help with something that isn't explained [well enough] here.
A. Scroll to the bottom of this document for contact information.


----=Files=----

/
Readme.txt     This file

Commands.txt   Bot commands
Operators.txt  Bot operators
Spawns.txt     Bot names
MERVBot.INI    Other bot settings
MERVBot.exe    MERVBot application
subspace.bin   Checksum data file
zlib.dll       File transfer library
Other *.dll    Bot spawn definitions

/bak
*.bak          Documentation backups

/src
zlib.lib       Linkable object library
*.h            Bot source; declarations
*.cpp          Bot source; definitions
MERVBot.vcproj VC++ 7.0 project file
MERVBot.ds*    VC++ 6.0 project files

/lvl
*.lvl          Level files

/get
*.*            *getfile folder
news.txt       Zone news

/log
*.log
Changelog.txt  History and future
Logins.txt     History of logins


----=Contact the Author=----

mailto:cat02e@fsu.edu
ICQ#18736684
SubSpace:CAT ASSS Test Zone
IRC:irc.openprojects.net #sheepcloning #C++
http://mervbot.com
^^ Before reporting a bug, download the latest version from my site.

i've been accused of being a poet. -Interpretive
