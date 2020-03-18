GAPI implementation for HP Jornada 720 HandheldPC

See license.txt for licensing terms.

NOTES:
This implementation uses BitBLT engine found in the Epson S1D13506
video controller to implement double-buffered video mem on J720.




-----------------
HACK NOTES:
Some applications that use 320x240 graphics will center the screen
when detect that the screen is actually 640x240 (like n0p's DOSBox port).

n0p's port actually have a useful soft-keyboard drawn in the bottom of
the screen that provides keys not found on Jornada 7xx devices (F1,F2...)
but it's totally unusable because pDosBox expects the screen to start
at the very left edge of the screen and since the screen is centered
(shifted by 160 pixels) so will be the "tap" coordinates.
I hacked this GAPI Implementation to report the video memory starting 
160 pixels before it actually does. That cause pDosBox to believe it's
writting at the center of the screen while is actually writting in the
very left edge.

This Hack may not be useful for tons of apps, but at least it gets the
job done with n0p's excellent port of DOSBox.

			Elias Zacarias - 08/20/2006