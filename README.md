# Stream Scope

Stream scope is supposed to be a little program to view the oscilloscope screen on your PC.  
This should work with VISA enabled USB oscilloscopes.  

For now I've only ever tested it with my DS1052E and a friend's DS1054Z.  

It has been made with the DS1052E in mind so there are, probably, bugs if you use it for other oscilloscopes.  
So it's far from perfect, don't expect many updates either.  

<img src="stream scope.png">

## Why:
----------

Because.  

Jokes aside, I do a bit of streaming on twitch from time to time and wanted to be able to show my oscilloscope's screen while measuring stuff on hardware.  
Having only one camera I can't properly show both at the same time, so streaming the oscilloscope screen is the best option.  

## Dependencies:
----------------

- SDL2
- SDL2 TTF
- VISA (from National Instruments)


## Compiling:
-------------

Use your favorite make and C compiler to build the program.