How To Use colourcal.ino

Output:
WHIVAL
BLAVAL
GREVAL
REDARR
GREARR
YELARR
PURARR
BLUARR


Main functions:
setBalance() - Retrieves analogRead RGB values from the LDR for both white and black being calibrated (prenormalisation)
getColour() - Gets RGB colour values of current colour and prints them (normalised)

This file is to be used with the Serial Monitor open.

In setup(), white and black are calibrated respectively using setBalance(). Take note of the RGB values from the LDR (prenormalised).
These values will go into WHIVAL and BLAVAL. GREVAL will be the difference.

In loop(), choose whichever colour to be placed over the LDR. RGB values will be printed using setColour(). Take note of these values (normalised).
These values will go into REDARR,GREARR,YELARR,PURARR,BLUARR.

That's it! No worries if it prints the wrong colour in loop(), just take note of the new calibrated values and test again.
Once the values are finalised, copy paste them into main.ino.