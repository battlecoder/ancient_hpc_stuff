OKey v 2.0 [2MyBeloved] Release
---------------------------------------------
				By Elias Zacarias
				08/21/2006


INTRODUCTION:
----------------------
  While Most desktop Operating Systems offer a way to change the mapping of
  your keyboard in order to support international charsets or even different
  keyboard layouts other than QWERTY, handheld devices does not provide any
  kind of mechanism for doing this. Of course they are some commercial
  applications that allow this but I have no way (nor interest) on adquiring
  one of them.
  With OKey you can remap your keyboard as you like, you can make X be Z and
  Z be Y and so on.
  Also, you can create special key combinations to generate UNICODE characters.
  As stated in their website (http://www.unicode.org):
  
  "Unicode provides a unique number for every character, no matter what the
  platform, no matter what the program, no matter what the language"
  
  UNICODE contains every existing character in known languages and symbol sets
  like mathematics and OKey will let you assign keys on your keyboard to make
  any char you want (E.g: You can setup Ctrl+A to be "a with umlaut").
  Of course, the font you are using must support the char in order to proper
  display on the screen but that's an obvious condition.

  There's a (mostly for memory issues) limit of 256 custom UNICODE combinations
  that can be expanded in future releases if needed.

  THANKS TO:
----------------------
 Thanks to Prathamesh Kulkarni for his code on Keyboard Hooking under Windows CE.
 Without his code this program wouldn't exist. ;)
 
 

DISCLAIMER:
----------------------
INSTALLING, DISTRIBUTING AND/OR USING THIS SOFTWARE IMPLIES THAT YOU HAVE
   READ THIS LICENSE AND UNDERSTAND AND AGREE ITS TERMS AND CONDITIONS.

   Usage:
   You can use this software freely with non-profit purposes only.

   Distribution:
   If you plan to distribute this package you must not charge for it except
   for the physical storage medium used to distribute it if any.
   If you have paid for this package for other reason other than this one,
   please contact your distributor in order to demand the fulfillment of this
   condition.
   However, an additional clause applies to distribution; files and directory
   structure of the package must be kept. Adding, deleting or otherwise modifying
   files and/or directories of the package violates this statement.

 WARRANTY:
    This program is provided WITHOUT ANY KIND OF WARRANTY. Damnsoft development team
    or any member of it have no responsibility about any damage made
    to you, or your computer and/or peripherals after/while/before using this
    program. Damnsoft is not guilty if your computer explodes, disappears and/or
    starts talking to you or otherwise your dog leaves home, your daughter's
    boyfriend gets involved on a crime, your wife is arrested, your boss
    reduce your salary payment or any event on your -or mankind- lifetime.




USAGE:
----------------------
About Key assignments
----------------------
  Every key on your keyboard has a code which ranges from 0 to 255. Since normal
  keyboards DOESN'T have more than 100 and a few keys, there are LOT of unused
  codes. OKey list EVERY single code, used or unused when asking you for keys and
  combinations. 
  THAT DOESN'T MEAN THAT ALL COMBINATIONS WILL WORK.
  (E.g: Alt+Any_number won't work since the operating system expects a code to be
  typed while holding Alt and typing numbers).
  Hardware Keys (usually quicklaunch keys) usually cause composite WIN+[CODE]
  combinations so even if OKey let you map them they won't work in most cases).

  The touchscreen "tap" action also have a code in this range and thus is listed
  on OKey BUT you are most likely to fail assigning ANY thing to LBUTTON.
  
  THEY ARE ALL INCLUDED FOR THE SAKE OF COMPLETENESS!! You need to test if your
  combinations actually work.
  
  
Setting it Up
----------------------
  When you launch OKEY you'll see the main window opened. It's actually self
  explainatory but I'll make a brief introduction.
  There's a box under "Layout file" on which you'll see the current Layout
  loaded (files with .OK extension). Next to it there's a "..." button that
  allows you to browse your device in order to load another layout.
  A Default.OK file is included in the standard distribution. This file has
  no combinations and the default mapping applied (A=A, B=B, C=C, etc).
  The button in the lower right corner of the dialog allows you to minimize
  OKey into the system tray.

  One important thing is that OKey will automagically DISABLE itself if any
  setup dialog is displayed. A small icon on your system tray will show you
  the current state of OKey (will appear greyed if disabled).
  
The Layout Editor
--------------------
  Here you can assign direct translations between keys. It's very easy to use.
  If for any reason there's a Key that isn't recognized by the Layout Editor when
  you press it, feel free to use the provided "Key simulation Tool" (Note that
  in most cases if the Layout Editor doesn't recognize the key is because is a
  system key that will mostly won't work on any combination).
  
  The "Save..." Button let you save your current layout (including UNICODE
  combinations) into a .OK file.
  
  The "Unicode Combinations" occupies the right part of the dialog.
  Here you can set up special combination or single keys to generate ANY UNICODE
  character.
  Next to the "Add" and "Del" button is the list of registered combinations you
  have made so far in this layout. As stated before, up to 256 combinations can be
  registered.
  
  At the rightmost part of the Dialog you can choose the font used for the UNICODE
  preview.
 

The Advanced Setup
------------------------
  You can make up to 10 layout files to be in the "quick access list" that is
  displayed when you tap on the systray icon while OKey is minimized for quick
  loading those layouts.
 
  Below the Layout List, there's the Device ON/OFF Key Selector.
  At least in the Jornada 720, the system NEEDS the on/off key to be pressed without
  further processing, so OKey needs to disable itself when the power key is pressed,
  and it'll enable itself again once its depressed (that's when you turn ON your unit
  again). 
  In this part you'll be able to setup the Power Key for your device if your device
  has the same behavior as the Jornada. Just press the "Setup New" button and then
  turn off your device. Once it's powered down turn it ON again.
  The Power Key should be set Now.
  By default is set to VK=223 (OEM_8) which is the code in the Jornada 720 HPC.

  Between the layout list and the PowerKey setup, you will found a checkbox
  labeled as "Never map ESC key". If checked, this option will cause the ESC key
  to be passed directly to the OS ignoring any mapping associated to it.
  Some apps require keys to be passed directly to the OS, that seems to be the case
  of popup menus that can be closed with ESC. Without this option checked, you
  won't be able to close popup menus with the ESC key.
  

Other Options
------------------------
 You can optionally set OKey to load the last layout you used automatically when
 launched by checking the "Auto load last layout" box in the main dialog.
 Also, you can associate OKey with the "*.ok" files. If you do so, OKey will setup
 itself to be the application launched when you double tap or open a *.ok file.
 


CONTACT INFO:
----------------------
Visit us at http://www.damnsoft.org to get the last versions, games and apps from DamnSoft.
Bug reports as well as suggestion and comments are welcomed at user_feedback@damnsoft.org.