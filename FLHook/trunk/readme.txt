F L   H O O K
=============
version: 1.6.6
Based on FLHook v1.5.5 with w0dk4's cloak code

NOTE: If you wish to use this version of FLHook in place of your standard 1.5.5,
please read the changelog.  The default FLHook.ini works with vanilla FL; for
an example of a config file for a mod, look at FLHook Flak.ini ('88 Flak's
config file).

================================================================================
== INSTALLATION ================================================================
================================================================================
FLHOOK ONLY WORKS WITH FLSERVER 1.1. USING IT WITH 1.0 WILL CRASH FLSERVER!!!

copy the files from "bin" to your freelancer/exe directory and edit the FLHook.ini
in order to suite your needs.
there are 3 different ways to start flhook:
1) open dacomsrv.ini in "...\freelancer\exe" and append FLHook.dll to the 
   [Libraries] section. flserver will load flhook whenever you start it.
2) execute FLHookStart.exe while flserver.exe is already running. this will
   inject FLHook into the running process.
3) execute FLHookStart.exe with "execute ..." as command line. this will start
   flserver.exe along with FLHook. the rest of the command line will be passed
   on to the server. FLHook console will be shown as soon as the loading-process
   has finished
(note: the old patch is not needed anymore, use 1) instead)

================================================================================
== CONFIG ======================================================================
================================================================================
take a look at the comments in FLHook.ini

================================================================================
== ADMIN-COMMANDS ==============================================================
================================================================================
commands can be executed by administrators in several ways:
- using the FLHook console(-> access to all commands)
- ingame by typing .command in the chat(e.g. .getcash Player1)
  this will only work when you own the appropriate rights which may be set via
  the setadmin command. FLHook will store the rights of each player in his
  account-directory in flhookadmin.ini.
- via a socket connection in raw text mode(e.g. with putty)
  connect to the port given in the FLHook.ini and enter "PASS password". after
  having successfully logged in you may enter all of the commands you want(as
  long as you have the necessary rights). you may have several socket connections
  at the same time. exiting the connection may be done by entering "quit" or simply
  by closing it.

- CASH -
all cash functions work no matter if the player is currently logged in or not
getcash <charname>
  shows current account balance of <charname>
setcash <charname> <amount>
  sets current account balance of <charname> to <amount>
setcashsec <charname> <oldmoney> <amount>
  sets current account balance of <charname> to <amount>, only works when his old account balance is <oldmoney>
addcash <charname> <amount>
  adds <amount> to the current account balance of <charname>
addcashsec <charname> <oldmoney> <amount>
  adds <amount> to the current account balance of <charname>, only works when his old account balance is <oldmoney>

- KICK/BAN -
kick <charname> <reason>
  disconnects <charname>. the user will be displayed <reason>, if it is specified.
ban <charname>
  bans <charname>'s account
  <charname> stays connected if he's currently on the server
unban <charname>
  unbans <charname>'s account
kickban <charname> <reason>
  kicks and bans <charname> (2 in 1, same as kick <charname> <reason>, ban <charname>)

- MSG -
msg <charname> <text>
  private message <text> to <charname> (shown as "Console: <text>")
msgs <systemname> <text>
  send <text> to all players in <systemname> (shown as "Console: <text>")
  <systemname> must be the either the system-id or the shortname (like Li01)
msgu <text>
  message <text> to the whole universe
msguc <text>
  message <text> to the whole universe (shown as "Console: <text>")
fmsg <charname> <xmltext>
  private message <xmltext> to <charname> (see XMLTEXT section for further details)
fmsgs <systemname> <xmltext>
  send <xmltext> to all players in <systemname> (see XMLTEXT section for further details)
fmsgu <xmltext>
  message <xmltext> to the whole universe (see XMLTEXT section for further details)

- BEAM/KILL -
beam <charname> <basename>
  force <charname> to land on <basename> (player must be in space)
  <basename> must be either the shortname(like Li01_01_Base for Manhattan) or a shortcut defined in the FLHook.ini
  Player may be kicked after beam
  note: see the issues section below
kbeam <charname> <basename>
  Kills <charname>, respawning them at <basename> when they click RESPAWN
  As long as the player respawns after dying, they will not lose any cargo
  This is meant as a replacement for beam, which sometimes kicks a player
kill [charname]
  kills [charname] if it is defined, otherwise kills object selected in HUD
instantdock <charname>
  Instantly docks <charname> to the selected object in HUD; note that the object and <charname> must be in the same system

- REPUTATION -
resetrep <charname>
  sets <charname>'s reputations to the one specified in "mpnewcharacter.fl"
setrep <charname> <repgroup> <value>
  set <charname>'s reputation for <repgroup> to <value>. <value> should be between -1 and 1.
  example:
  "setrep playerxy li_n_grp 0.7"
  -> set playerxy's reputation for liberty navy to 0.7
setreprelative <charname> <repgroup> <value>
  Adjust <charname>'s reputation with <repgroup> by <value>.
  Works like when you kill a NPC - the reps with all the other factions are adjusted as well.
getrep <charname> <repgroup>
  get <charname>'s reputation for <repgroup>.
  getrep playerxy li_n_grp returns "rep=0.1223", for example.
getaffiliation <charname>
  gets <charname>'s affiliation; returns "repgroup=li_n_grp", for example.
setaffiliation <charname> <repgroup>
  sets <charname>'s affiliation

- CARGO -
All cargo commands except for addcargo only work when the targeted player is ingame
enumcargo <charname>
  lists <charname>'s cargo, first reply will be the remaining hold size
addcargo <charname> <good> <count> <mission>
  adds <count> numbers of <good>(shortname like co_gun01_mark02,commodity_silver,etc OR hash) to <charname>'s cargo
  if <mission> is set to 1, the cargo is declared as mission cargo
addcargom <charname> <good> <hardpoint> <mission>
  Same as above except the cargo is mounted to <hardpoint>.
removecargo <charname> <id> <count>
  removes <count> numbers of <id>(this must be the value from enumcargo's "id=" reply) from <charname>

- CHARACTERS -
rename <oldcharname> <newcharname>
  rename <oldcharname> to <newcharname> (player will be kicked if he's logged in <oldcharname>'s account)
deletechar <charname>
  delete <charname> (player will be kicked if he's logged in <oldcharname>'s account)
readcharfile <charname>
  reads <charname>'s userfile(xxx.fl) and prints it(each line will be preceded by "l ")
writecharfile <charname> <data>
  writes <data> into <charname>'s userfile(xxx.fl). existing charfile will be overwritten. you should
  be careful with this one because a corrupted charfile may lead to server crashes and flhook does 
  not do any syntax checks on <data>.
  NOTE: YOU MUST REPLACE LINE-WRAPS WITH \n(TEXT)
  example:
  writecharfile playerxy [Player]\ndescription = 00300034002f0031003\n\n ... etc.

- SETTINGS -
setadmin <charname> <rights>
  set <charname> as ingame-admin with <rights> (affects all characters on the account) (see RIGHTS section below)
getadmin <charname>
  show <charname>'s rights
deladmin <charname>
  revoke <charname>'s ingame-admin-status
setadminrestriction <charname> <restriction>
  Only allow <charname> to execute admin commands on characters with names that include <restriction>.
getadminrestriction <charname>
  Gets the admin restriction for <charname>; returns "restriction=[blarg]", for example.
rehash
  reload the flhook.ini in order to activate changed settings, this works for everything except the socket-settings
unload
  unload flhook(flserver will keep on running flawlessly)
  this is very useful since it allows you to install a new flhook version on-the-fly. simply unload, replace the files
  and execute FLHookStart.exe
  the command will kick players with an active moneyfixlist(don't bother).

- GROUP -
getgroupmembers <charname>
  returns all players which are in a group with <charname>
getgroup <charname>
  gets the group-id of <charname>; returns "groupid=2", for example.
  players in no group have a group-id of 0.
addtogroup <charname> <group>
  adds <charname> to <group>
invitetogroup <charname> <fromcharname>
  invites <charname> to join <fromcharname>'s group
removefromgroup <charname> <group>
  removes <charname> from <group>
sendgroupcash <group> <amount>
  sends everyone in <group> <amount> of cash, divided among the members

- DEATH PENALTY -
These commands require both cash and cargo rights.
setdpfraction <decimal amount>
  Sets the death penalty fraction
adddpsystem <systemname>
  Add a system to the death penalty exclusion list
removedpsystem <systemname>
  Remove a system from the death penalty exclusion list
enumdpsystems
  Print all systems on the death penalty exclusion list
  
- OTHER -
getbasestatus <basename>
  returns the hull status of a base. when the base hasn't been created in space yet it returns 0.
getclientid <charname>
  gets <charname>'s client id
getplayerinfo <charname>
  get <charname>'s info
xgetplayerinfo <charname>
  same as getplayerinfo, except that result is shown in a more readable format
getplayers
  get player info for all players on the server (players in charselect menu will not be shown)
xgetplayers
  same as getplayers, except that result is shown in a more readable format
getplayerids
  shows all players on the server with their client-id in a short format(useful when ingame)
getaccountdirname <charname>
  get account-dirname of <charname>
getcharfilename <charname>
  get char-filename of <charname>
savechar <charname>
  save <charname>'s current info to disk
isloggedin <charname>
  check if <charname> is logged in on the server
isonserver <charname>
  check if <charname> is connected(this includes idleness in charselect menu) to the server
  NOTE: isonserver will also return true, when another char on the same account is logged in!
serverinfo
  shows server load, whether npc spawn is currently enabled or disabled(see ini) and uptime.
  the format for the uptime is: days:hours:minutes:seconds
moneyfixlist
  show players with active money-fix
restartserver
  restart the server; command line params can be set in ini.  NOTE: will create an instance 
  of cmd.exe, which will exit when the server does; requires that the server have
  taskkill installed (is not installed by default on XP Home).
shutdownserver
  shutdown the server.  NOTE: requires that the server have taskkill installed (is not installed
  by default on XP Home).
spawns [on|off]
  Turns NPC spawns on or off.
help
  get a list of all commands
  
- MISC INFOS -
you can use client-ids instead of <charname> by appending $ to the cmd.  Using $ for the client-id means use your own.
examples: 
getcash$ 12
kickban$ 1
getplayerinfo$ $
etc.

you can also use "shortcuts" instead of the whole character-name for a currently logged in player by appending
& to the cmd. FLHook traverses all logged in players and checks if their character-name contains
<charname>(case insensitive) and if so, the command will operate on this player. an error will be shown if the 
searchstring given in <charname> is ambiguous.
examples (let's assume there are 2 players logged in: "superhax0r" and "..::[]SUPERNERD[]::.."):
"kick& nerd" kicks "..::[]SUPERNERD[]::.." because his nick contains "nerd"
"getcash& super" fails because there are multiple character names containing "super"

all commands return "OK" when successful or "ERR <errortext>" when error occured

================================================================================
== RIGHTS ======================================================================
================================================================================
rights may be seperated by a comma (e.g. setadmin playerxy cash,kickban,msg)
superadmin        -> everything
cash              -> cash commands
kickban           -> kick/ban/group commands
beam              -> beam/kill/resetrep/setrep command
msg               -> msg commands
savechar          -> savechar command
cargo             -> cargo commands
chars             -> rename/deletechar/readcharfile/writecharfile
eventmode         -> eventmode (only when connected via socket)
all other commands except setadmin/getadmin/deladmin/restartserver/shutdownserver/spawns may be executed by all admins

================================================================================
== XMLTEXT =====================================================================
================================================================================

the fmsg* commands allow you to format text in several ways(like in the exe\misctext.dll)
text is enclosed in <TEXT></TEXT> tags while the format can be changed with <TRA .../>
nodes-names must be written in capital chars! 
be sure to replace the following characters within a text-node:
< -> &#60; 
> -> &#62;
& -> &#38;

- <TRA .../> NODE SYNTAX -
the data field of a TRA node consists of an RGB value along with format specifications:
<TRA data="0xBBGGRRFF" mask="-1"/>
BB is the blue value
GG is the green value
RR is the red value
FF is the format value
(all in hexadecimal representation)

format flags are:
bin      hex  dec  effect
00000001 1    1    bold
00000010 2    2    italic
00000100 4    4    underline
00001000 8    8    big
00010000 10   16   big&wide
00100000 20   32   very big
01000000 40   64   smoothest?
10000000 80   128  smoother?
10010000 90   144  small
simply add the flags to combine them (e.g. 7 = bold/italic/underline)

examples:
fmsgu <TRA data="0x1919BD01" mask="-1"/><TEXT>A player has died: Player</TEXT>
this is similar to the standard die-msg(which is shown in bold)
fmsgu <TRA data="0xFF000003" mask="-1"/><TEXT>Hello</TEXT><TRA data="0x00FF0008" mask="-1"/> <TEXT>World</TEXT>
this will show "Hello World" ("Hello" will be blue/bold/italic and "World" green/big)

================================================================================
== EVENTMODE ===================================================================
================================================================================
socket connections may be set to eventmode by entering "eventmode". from then on 
you will receive several event-notifications listed below. once activated, 
eventmode runs until you close the connection.

- NOTIFICATIONS -
chat from=<player> id=<client-id> type=<type> [to=<recipient> idto=<recipient-client-id>] text=<text>
  <player>: charname sending the message
  <client-id>: client-id of sender
  <type>: either universe,system or player
  <recipient>/<recipient-client-id>: only sent when type=player
  <text>: guess ...

kill victim=<player> by=<killer1>[,<killer2]... cause=<killer1's weapons>[;killer2's weapons]...
  <player>: charname of the victim.
  <killerx>: charname/faction(if NPC) of the killer; sorted from highest to lowest damage.
  <killerx's weapons>: comma seperated list of weapons used to kill player.  Weapons can be guns, missiles (only if CombineMissilesTorps is set to no), mines, torpedoes (only if CombineMissilesTorps is set to no), missiles/torpedoes (only if CombineMissilesTorps is set to yes), or collisions.  Sorted from highest to lowest damage.

login char=<player> accountdirname=<dirname> id=<client-id> ip=<ip>
  occurs when player selects a character in the character-select menu

launch char=<player> id=<client-id> base=<basename> system=<systemname>
  occurs when player undocks from a base/planet

baseenter char=<player> id=<client-id> base=<basename> system=<systemname>
  occurs when player enters base
  
baseexit char=<player> id=<client-id> base=<basename> system=<systemname>
  occurs when player exits base(includes disconnect/f1)

jumpin char=<player> id=<client-id> system=<systemname>
  occurs when player jumps in a system
  
switchout char=<player> id=<client-id> system=<systemname>
  occurs when player switches out a system

spawn char=<player> id=<client-id> system=<systemname>
  occurs when player selects a character and launches in space

connect id=<client-id> ip=<ip>
  occurs when player connects to the server

disconnect char=<player> id=<client-id>
  occurs when player disconnects from the server

================================================================================
== USER-COMMANDS ===============================================================
================================================================================
user commands may be entered ingame by every player in chat and can be enabled
or disabled in the ini. enter them ingame to get a description.

/set diemsg xxx
  while xxx must be one of the following values:
  - all = all deaths will be displayed
  - system = only display deaths occurring in the system you are currently in
  - self = only display deaths the player is involved in(either as victim or killer)
  - none = don't show any death-messages
  settings keep saved in flhookuser.ini and affect all characters on the account

/set diemsgsize <small/default>
  - change the size of the diemsgs

/set chatfont <size> <style>
  <size>: small, default or big
  <style>: default, bold, italic or underline
  this lets every user adjust the appearance of the chat-messages

/autobuy <on|off>
  - on: saves the current configuration of unmounted items and attempts to
  complete that configuration upon entering a base.
  - off: deletes the current configuration.

/ignore <charname> [<flags>]
  ignore chat from certain players

/ignoreid <client-id> [<flags>]
  ignore by client-id
  
/delignore <id> [<id2> <id3> ...]
  delete ignore entry
  
/ignorelist
  display ignore list
  
/ignoreuniverse <on|off>
  Ignores chat from the universe channel

/ids
  show client-ids of all players

/id
  show own client-id
  
/i$ <client-id> and /invite$ <client-id>
  invite player to group by client-id

/cloak and /c
  Cloak the ship if a cloaking device is present on it.
  If used when cloaked, shows the time remaining for the cloak.

/uncloak and /uc
  Uncloak the ship if it is cloaked.

/rename <new name>
  Renames the logged-in character to the new name.  Costs an amount of credits set in the ini.

/sendcash <charname> <amount>
  Sends <amount> of credits to <charname>.  Charges a tax set in the ini to the sender.

/sendcash$ <client-id> <amount>
  Same as above but with client-id.
  
/afk [Message]
  Sets away from keyboard message, which is automatically sent to people who message you.
  The default message is "I'm away from my keyboard right now."
  If no message is appended to the command and AFK is set, then AFK is unset; 
  otherwise the message is updated with the new message specified.
  
/inviteall [name part] and /ia [name part]
  Invites all players on server to join into a group with you.
  If [name part] is present, it only invites those who have [name part] in their name.

/shieldsdown and /sd
  Makes the shields on your ship fail.
  
/shieldsup and /su
  Makes the shields on your ship recharge to the levels they were at when the shields
  down command was used.  Note that it will not make your shields go up instantly.
  
/mark and /m
  Makes the selected object appear in the important section of the contacts and
  have an arrow on the side of the screen, as well as have > and < on the sides
  of the selection box.
  
/unmark and /um
  Unmarks the selected object marked with the /mark (/m) command.
  
/groupmark and /gm
  Marks selected object for the entire group.
  
/groupunmark and /gum
  Unmarks the selected object for the entire group.
  
/ignoregroupmarks <on|off>
  Ignores marks from others in your group.
  
/automark <on|off> [radius in KM]
  Automatically marks all ships in KM radius.  Bots are marked automatically in the
  range specified whether on or off.  If you want to completely disable automarking,
  set the radius to a number <= 0.
  
/botcombat <on|off>
  Turns your bots (AI Companion option) hostile to you for combat purposes.
  
/dock and /d
  Docks your ship at a valid player ship, allowing you to repair and buy equipment
  as with a normal base.
  
/transfer <charname> <item>
  Transfers <item> to <charname>.  Valid items are those that are not grouped
  (like commodities and ammo).  Charges a price set in the ini to the sender.

/transfer$ <client-id> <item>
  Same as above but with client-id.
  
/enumcargo
  Prints out a number for each cargo item that can be used with the /transfer command.

/dp [on|off]
  Shows information about the death penalty.  Also sets whether a notice about how
  much the death penalty costs is shown upon launch.
  
/help [command] and /? [command]
  If command is not specified, prints out list of commands.  If it is specified,
  prints out information on command.

================================================================================
== LOGGING =====================================================================
================================================================================
flhook logfiles are created in "My Documents\My Games\Freelancer\Accts\MultiPlayer".

flhook_kicks.log:
  idle-/ping-/loss-/corruptedcharfile-/ipban-/hostban-kicks
flhook_cheaters.log:
  detected cheating

there is also a flhook.log file in the exe dir. it mainly contains debug data.
so don't care about this one.

================================================================================
== SPECIAL FEATURES ============================================================
================================================================================
- Cloak by w0dk4: adds a balanced cloaking system to FLHook. Cloaking devices 
  and cloaking times are defined in the ini under [CloakDevices].
- Repair guns: guns that repair instead of damage ships and solars. They are 
  defined in the ini under [RepairGuns].
- Alternate base repairing: bases whose hit points match the list defined under
  [BaseRepair] in the ini are not repaired for the first half the time set and 
  then are repaired completely during the second half of the set time. The time 
  can be set in the ini under [General]; it is 240 seconds by default.  If the
  base is killed while being repaired the cycle restarts at the end of the set
  time.  The base can only be repaired by its hit points, so if it is damaged
  while being repaired it will not be repaired all the way.
- Automatic changing of affiliation: when the mountable faction tags present in
  the 88 Flak mod are bought from a base and are mounted automatically after 
  being bought, the affiliation of the player who bought the tag is changed to
  that of the tag. Note that for this to work the nickname of the faction tag 
  must have "_grp_token" in it, be the nickname of the faction it represents 
  with a "_token" appended to it, and be defined in misc_equip.ini.
- Mobile docking: allows players to dock with other players.  The valid ships
  can be set under [MobileBases] in the ini.  Ships cannot dock with other ships
  on the list.  In order for mobile docking to function, there must be a base in
  every system with a nickname and base name of <system nick>_mobile_proxy_base.
  Players will be instantly docked at this base when /dock (/d) is executed on a
  valid ship and that ship is in range (settable in ini under 
  [General]->MobileDockRadius).  The distance under or over the carrier the
  player is launched at can be configured in the ini under
  [General]->MobileDockOffset.  Players must be in the same group in order to dock.

================================================================================
== KNOWN ISSUES ================================================================
================================================================================
- crashes/performance:
  FLHook is very resistant to server crashes. we've had it running on hhc for
  weeks without any major problems. however in case you encounter any crashes
  which are obviously related to FLHook please contact me.
- the standard beam command can cause the server to crash.  This is why this
  version of FLHook has a setting allowing configuration of the .beam command.
  By default it is set to use the improvements made by w0dk4 in the plugin
  version.  Look at [General]->BeamCmdBehavior for the setting and more
  information.
- the sender in the standard chat messages appear a little bit smaller when
  "/set chatfont" is enabled
- calculation of the "loss" has been improved but is still problematic. players with
  low bandwidth tend to have a very high loss while they're loading, so this may
  lead to some problems. i don't recommend using the loss-kick at this moment.

================================================================================
== SOURCE CODE =================================================================
================================================================================
FLHook compiles on both vc7 and vc6
Note:
flserver uses string-class arguments in some of its functions(f. e. 
PlayerDB::FindAccountFromCharacterName(...)). when you compile since FLHook with 
vc7, which uses a different stl, you can't simply pass strings. that's why
i created the FLHookWString.dll which was compiled with vc6 and just exports
two functions to create/delete flserver compatible strings. if you don't have
vc6 installed then use the FLHookWString.dll from bin and thats it.

if you want the full header files/libs of all of the relevant flserver dlls then 
take a look at FLCoreSDK(www.skif.be). it contains everything you need.
Since www.skif.be has disappeared, w0dk4 has mirrored the DL at
http://aa-flserver.de/download/FLCoreSDK.zip .

please note:
I WILL NOT TEACH YOU HOW TO CODE, I WILL NOT TELL YOU HOW TO REVERSE FLSERVER 
AND I WILL NOT REPLY TO "STUPID" QUESTIONS REGARDING THE SOURCE - mc_horst
w0dk4 and M0tah however are generally willing to try and answer questions regarding
the source/FLHook in general.  The FLHook forum can be found at
http://flhook.drachennest200x.de/forum/ .

================================================================================
== BLOWFISH ENCRYPTION PROGRAMMING =============================================
================================================================================
Because Blowfish only supports encrypting 8 bytes of data at once, it is
necessary to pad data (with 0x00s) that is not a multiple of 8 bytes.  When
recieving encrypted data from FLHook, you should check for null characters and
remove them from the decrypted string.  When sending encrypted data to FLHook,
you should pad the string with null characters if necessary.  If the blowfish
implementation you are using specifically encrypts and decrypts strings, this
may already be handled by it.

================================================================================
== CREDITS =====================================================================
================================================================================
unofficial builds after 1.5.5 programmed and documented by M0tah
Thanks to w0dk4 for answering questions I've had regarding hooks and other things
pertinant to FLHook and for coding a fix for the beam command - M0tah
Anti-Superjump code taken from http://flhook.drachennest200x.de/forum/viewtopic.php?t=11,
thanks to Reaper.
PlayerDB struct update taken from http://flhook.drachennest200x.de/forum/viewtopic.php?t=344,
thanks to Devast8or.
Beam command fix taken from the plugin version, programmed by w0dk4.  It can be
found at http://the-starport.net/index.php?option=com_smf&Itemid=26&topic=523.0 .
w0dk4's socket buffer overflow fix has also been implemented.

programmed & documented by mc_horst
BIG THX to w0dk4 and Niwo for testing and the whole Hamburg City Server adminteam 
for the suggestions and feedback

contact: find me on www.freelancerserver.de or #hc-flserver (quakenet)

FLHook uses a slightly modified version of flcodec.c

Blowfish encryption was implemented using the C algorithm by Paul Kocher,
downloaded from http://www.schneier.com/blowfish-download.html

================================================================================
== CHANGELOG ===================================================================
================================================================================

1.6.6 (unofficial)
- Re-implemented the kill event (for the socket eventmode)
- Autobuy checks if the number of nanobots and shield batteries in the template are greater than allowed for the ship
- Excess nanobots and shield batteries are sold after buying a new ship
- Added setting to give winners in PvP part of the loser's death penalty
- Changed cash printouts in DP code to use ToMoneyStr for better formatting
- Repair guns now work on external equipment on both NPCs and players
- Scripting no longer stacks instances in multiplayer
- Major reworking of config file organization into multiple files (still supports reading everything from FLHook.ini, however)
  - If you're lazy and don't want to reorganize your config, simply merge the settings in [Death] into [General]
- Added list of systems where items of a certain type cannot exist in space or be traded

1.6.5 (unofficial)
- Fixed getting account dir failing on some computers (http://the-starport.net/f/index.php?topic=853.0)
- Added ship repairing based on type of ship or presence of mounted item (see [ItemRepair] and [ShipRepair] in ini)
- Added blowfish encryption option for the socket connection
- Fixed addcargo kicking upon launch when adding items not sold at base (bug introduced in 1.6.1 when addcargo was partially rewritten)
- Added new admin command addcargom that adds mounted cargo
- Added list of items to automatically mount upon ship purchase (see [AutoMount] in ini)
- Fixed docking restrictions cargo being removed when player canceled dock
- Fixed cash being deducted when attempt to buy item was blocked ([ItemRestrictions] in ini)
- Added list of items that cannot exist in space (see [NoSpaceItems] in ini)
- Added list of items that are automatically marked when they spawn (see [AutoMarkItems] in ini)
- Added setting to drop the shields of a ship when starting to warm up the cloak
- Changed autobuy to run when player launches to space (so player can sell loot before autobuy tries to buy items)
- Fixed DP not being charged if player launches in a system on the no DP list and then dies in a system not on the list
- Fixed DP not being charged if player turns off the DP notices (bug introduced in 1.6.3)
- Fixed players being able to remark a ship once it has started to cloak
- Fixed a mobile docking bug where players would follow the carrier's jumps if the carrier died and the players had not launched

1.6.4 (unofficial)
====
- Changed string resource reading code to use the LoadString WinAPI function instead of ResString
- Added setting for the amount rep is changed on death by PvP
- Ignore now functions on universe chat
- /ignoregroupmarks no longer toggles
- /ignoregroupmarks displays correct accepting/ignoring message
- Added spawns admin command
- Added /ignoreuniverse user command
- Added enumdpsystems admin command
- Added /me command

1.6.3 (unofficial)
====
- Fixed server crashes caused by FLHook (I hope..)
- Fixed autobuy behavior with multiple non-grouped items (thanks to Widowmaker for informing me of this bug)
- Autobuy no longer sells items if you have more than set
- Strings can now be read from resources.dll
- Fixed infinite loop in death message code
- Added a configurable list of items that cannot be traded between players (see [ExcludeTradeItems] in ini)
- Amount charged for DP uses amount calculated at launch
- Changed internal mount restrictions and docking restrictions to use binary trees for better performance
- Added additional options to docking restrictions, see ini for changes
- Added a configurable list of systems that the death penalty does not occur in and changed [ExcludeDeathPenalty] to [ExcludeDeathPenaltyShips]
- Added admin commands adddpsystem, removedpsystem, and setdpfraction
- Added an option to send system chat to universe instead
- Made the chat colors configurable
- There will be no more "Failed to hook content.dll" errors
- Added a suggestion to use the /enumcargo command if cargo is not found when using /transfer
- Updated the VC7 project file to include the files added since 1.5.5

1.6.2 (unofficial)
====
- Corrected the /help entry on /dp
- Fixed the reading of the CombineMissilesTorps setting
- Updated the IDS reading code to support extended character sets
- Fixed death penalty cargo removal
- Added the beam command fix from the plugin version, coded by w0dk4
- Added a setting for what is done for the .beam command
- Made .u a shortcut for .msgu
- Added % chance to the /dp items list
- Major marking code cleanup
- Players are unmarked upon cloaking
- Repair turrets now repair equipment/subobjects
- Repair turret damage now read from memory
- The filename of FL's config file is read from memory and parsed with INI_Reader
- Added setting to change the autopilot margin of error
- Removed non working code related to showing a player's faction in the contact list
- Added an option to have NPCs be damaged by collisions
- Damage by objects that have no affiliation is treated as erroneous except if caused by mines when formulating death messages
- Minor docking code fixup, should correct the functioning of <max number of ships that can be docked at once> and also might improve general stability of the mobile docking system
- Added support for up to 200 players

1.6.1 (unofficial)
====
- The HkAddCash method is now case-insensitive, fixing issues with adding cash by char name
- Entirely rewrote the algorithm for the FLHook death messages, making it more accurate and detailed
- Added in functionality for reading IDSs, related to above rewrite
- When a player is denied docking to a station because of its health, an additional message is sent to them as to the reason of the denial
- Added experimental code to prevent the spinning of large ships
- The radius of a ship is now taken into account when testing to see whether a player should be allowed to dock at a mobile base
- Autobuy command partially rewritten; is now much more flexible
- MountRestrictions are taken into account when buying a ship
- The enumcargo command now shows the name of the items instead of the hash
- Added a /transfer command and /enumcargo command for use with /transfer
- The addcargo admin command now works on offline characters
- The addcargo command will not add cargo unless it will fit in the target's cargo hold
- Added an option to make rep be changed from PvP deaths, as if an NPC of the killed player's faction was killed
- Added a setreprelative admin command, related to above
- Somewhat fixed the beam admin command - only kicks players sometimes
- Added charname restrictions to admin commands (useful for clan admins)
- All native FLServer commands (/group, /g, /system, /s, /universe, /u, /local, /l, /join, /j, /leave, /lv) are passed on
- Added death penalties
- Misc bug fixes, improvements

1.6.0 (unofficial)
====
- Repair guns now repair dead bases.
- /ignoreid changed to /ignore$
- /shieldsdown, /sd, /shieldsup, and /su commands added
- /help command added
- bug where "bla is now known as bla" messages did not display correctly fixed
- rename commands made more stable (no more weird, random bugs hopefully)
- GetAffiliation now works through player file reading
- New settings added to the ini - user commands made more configurable
- /mark (/m), /unmark (/um), /automark, /groupmark (/gm), /groupunmark (/gum), and /ignoregroupmarks commands added
- time elapsed before kick after beam increased to 1.5 seconds from 0.5
- got rid of unnecessary "OK" messages in player commands
- added docking restrictions - see ini
- added internal equipment mounting restrictions - see ini
- added low-health docking denied option
- added section in ini to specify items that change affiliation of player to that of the base when muonted
- bit of a source code cleanup
- /ia and /i$ now work on players with '<', '>', and '&' in their names
- If hooking fails, it is printed to the console
- Players made invincible while traveling though jump tunnels (should fix problems with being damaged by mines while jumping)
- /invite and /i commands restored to working condition
- $ now functions as a shortcut for inserting own client-id in admin commands (ex: .beam$ $ Planet Manhattan)
- /bountyhunt removed (it was buggy and I haven't gotten around to rewriting it)
- Anti Super-jump added from http://flhook.drachennest200x.de/forum/viewtopic.php?t=11
- restartserver and shutdownserver now check for the presence of taskkill.exe before proceeding
- No-PvP tokens can be defined in the ini now
- Players now cannot tractor loot once they have died (but before they have exploded)
- Non-solar objects cannot be repaired after they have died
- The alternate solar repairing now continues to repair the solar when it is killed while being repaired.
- shortcuts for cloaking added - /c and /uc
- kbeam added as admin command and should be more reliable than regular beam
- w0dk4's cloaking code made more configurable, see ini.  /cloak also made more informative.
- added an "Is88Flak" option in ini
- added /botcombat, which turns your AI Companion Bots (not wingmen) hostile to you
- The alternate solar repairing now works on all damagable solars
- Small fixup for death messages (should not erroneously print killed by admin messages)
- added mobile docking, see ini for config.  Thanks to w0dk4 for the LaunchPosHook.
- added time played restriction setting for /sendcash
- added admin command instantdock
- the kill admin command kills what you have targeted if no arguments are passed to it
- The help admin command fixed to have "pages" of commands
- Base-idling takes into account the player moving around/chatting

1.5.8 (unofficial)
====
- .restartserver and .shutdownserver commands improved
- weird bugs in the /sendcash command fixed and it now works as intended
- new command /afk added
- new commands /inviteall and /ia added
- new group admin commands added - see above.
- new commands for managing affiliation.
- admin command parsing is no longer activated when there is more than one '.'s in a row
- players are kicked from the server after being beamed to prevent server crashes
- damagable bases are destroyed for 2 minutes and then are repaired over the course of 2 more minutes.
- repair guns no longer repair equipment mounted to space objects; instead they only repair the ship even if they hit a subcomponent.  This fixes subcomponents being overrepaired until M0tah can figure out how to get the max health of said subcomponents.
- repair gun damage parsing code improved
- The mountable faction magnets in the 88 Flak mod now change the affiliation of the player when they are mounted when bought.
- The msgu command now functions identically to the /universe command in vanilla FL.  The old command was preserved under msguc.

1.5.7 (unofficial)
====
- Repair guns can now be specified in flhook.ini, more information can be found there
- new commands .restartserver and .shutdownserver

1.5.6 (unofficial)
====
- cloak added by w0dk4, look at flhook.ini for more information.
- BountyHunt taken from Leipzig City FLHook and integrated in.
- new commands /sendcash, /rename, and .getrep.

1.5.5 (official)
=====
- bug: rename bug fixed

1.5.4
====
- bug: /g fixed
- bug: deletechar fixed

1.5.1
=====
- ini: bans added

1.5.0
=====
- cmd: setrep added
- source: LOG_* macros removed
- source: compiles with vc6 now! (thanks to Mischa)
- bugfix: FLHookStart works on Win2k now(thx to Mischa)
- usercmd: /ignoreid, /ids, /id, /i$, /invite$ added

1.4.9
=====
- renamebug fixed: renamed chars are now shown in the account list of flserver
- bugfix: autobuy now checks if player has neccessary rep
- ini: MultiKillMessages added
- usercmd: "/set diemsgsize" added
- cmd: resetrep added
- cmd: getgroupmembers added
- ini: maximum groupsize can be altered

1.4.7
=====
- autobuy fixed
- disconnectrelay fixed
- ChangeCruiseDisruptorBehaviour fixed

1.4.6
=====
- usercmd: /autobuy added
- usercmd: /delignore may be called with multiple ids now

1.4.4
=====
- "reserved slots" added
- ini: DisconnectDelay added
- cmd: getbasestatus added
- eventmode: basedestroy added

1.4.3
=====
- usercmd: /ignore, /ignorelist, /delignore added

1.4.2 (official)
=====
- minor changes

1.4.1
=====
- kill command added
- ini: DeathMsgTextAdminKill added
- bug: security bug fixed(thx to ET90)
- fix: memory leak in getplayers/getplayerinfo fixed

1.4.0
=====
- typos in death msg fixed
- ini: new [Style] settings added
- "connect" event added
- cmd: serverinfo shows uptime
- new commands: xgetplayers/xgetplayerinfo
- ini: AntiF1 added

1.3.9
=====
- bugfix: /s bug fixed
- source-code cleanup
- anti-cheat check("Negative good-count") also when character logs in
- cmd: new feature called "shortcut"(very useful when command targets ingame player)
       see bottom of "ADMIN-COMMANDS" section for explanation
- unload automatically kicks players with moneyfixlist
- cmd: serverinfo added
- kickmsg added, you can alter the appearance of the reason in the ini
- you can now enter a reason in the kick/kickban command
- ini restructured (plz take a look at it!)
- ini: UserCmdStyle/AdminCmdStyle added to modify the appearance of ingame command-replies
- new eventmode notifications

1.3.8
=====
- flhook tries to detect corrupted charfiles and kicks player(notification will be added to flhook_kicks.log)
  if you have a charfile that still crashes the server then let me know
- ini: DisableNPCSpawns added

1.3.7
=====
- kicks get logged to flhook_kicks.log
- lossdata measuring-intervall set to 4sec
- rename bug fixed

1.3.6
=====
- cmd: rename/deletechar added
- anticheat: "selling more good than possible" detected(notification will be added to flhook_cheaters.log and player will get banned)
- ini: pingkick/losskick added
- cmd: "loss=" in getplayers now shows a reasonable result(it's an average value calculated by LossKickFrame setting)

1.3.5 (official)
=====
- bugfix: deathmsg was shown twice when "/set diemsg" was disabled

1.3.4
=====
- /u universe message bug fixed

1.3.3
=====
- rights: "setting" removed(was equal to superadmin)
- closing the flhook console will unload flhook(no more crashes)
- ini: wport option added for unicode socket connections
- ini: user commands may be enabled/disabled

1.3.2
=====
- cmd: getplayerinfo added
- cmd: getplayers shows ip/loss (not tested yet)
- beam exception leads to player kick(the cmd is save now)
- cruise disruptor behaviour may be changed(see ini)

1.3.0
=====
- IServerImpl now entirely hooked in HkIServerImpl.cpp
- cmd: getplayers shows ping

1.2.9
=====
- usercmd: "/set chatfont" added
- submitchat exception fixed?
- killmsg: death-fuse issue solved
- savechar after trade (to prevent cheating when server crashes)
- anti-baseidle added
- cmd: addcargo/removecargo/enumcargo added
- source-code cleanup
- bug: addcash/getcash now work with clientid as argument

1.2.8
=====
- ingame admin-command replies shown in a new format
- new readme.txt
- cmd: ban/unban changed
- ini changed
- print functions enhanced
- cmd: moneyfixlist added

1.2.7
=====
- bug: getcash bug(player in charmenu) fixed

1.2.6
=====
- bug: /leave bug in msg-cmd fixed 
- bug: special-char bug in socket connection fixed 
- killmsg: shows "xxx has died" when freighter with death-fuse was destroyed
- cmd: addcash/setcash now work flawlessly without kicks

1.2.5
=====
- xml format docu enhancements (however color="#..." doesnt work correctly)
- eventmode: login shows clientid
- diemsg bug fixed(player with <,> or & in their name had no die-msgs)
- cmd: isloggedin now returns false when player is in charmenu
- cmd: addcash/setcash now kick when player is logged in with a different char from same the account
- socket: bugfix, more than 1 cmd in one tcp packet now allowed (seperated by \n or \r\n)
- improved kill detection(now works with death-fuses?)

1.2.4
=====
- charfile encryption disabled

1.2.3
=====
- die-msg suppress fixed
- cmd: isloggedin added
- cmd: isonserver changed
- usercmd: /set diemsg added

1.2.2
=====
- fmsg added with xml-syntax
- eventmode: login added
- eventmode: chat spams id= and idto=
- try-catch statements added in callbacks
- release-mode-compilation finally works
- readme.txt added

