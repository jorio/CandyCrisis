- CANDY CRISIS NOTES -

- This project was originally developed using an open-source IDE called "DevCpp". It is 
  technically a front-end on gcc. DevCpp is not very elegant and it isn't always 100% 
  stable either, but it is free, open-source and much better than having no project manager 
  at all. After getting used to it, you can get a lot of work done in the environment and 
  (IMO) for a small project it was a better choice than spending $1000 on a license for 
  Dev Studio. If you already have Dev Studio, on the other hand, I'd recommend making new
  project files with Dev Studio and using that. It's a much more polished environment.

- When I worked, I put the code in a directory called "C:\Dev\" and had the IDE in a
  directory "C:\DevCpp\". If you move the code elsewhere, you will probably need to
  fix up all the project files. If you have Dev Studio and make your own project files,
  this is a non-issue.

- The installer was built with "Inno Setup" (www.innosetup.com), a surprisingly 
  functional yet free installer product. I last used Inno Setup in 2002, so the 
  existing "ISS" files may need to be redone to work with the latest Inno version.

- There is a makefile in the Source folder. At one point, with the assistance of a
  Linux-savvy coworker, this makefile could build a Linux version of the game. At
  this point I am not sure how much work is needed (if any) to get this makefile
  working again.


Here are the steps I took to make a final/shippable version of the game during its
development:

- Delete CandyCrisis.exe and rebuild the app fresh with DevCpp. (Deleting it is just
  paranoia, but probably best practice.)
- Compress the EXE with upx. (This makes the setup EXE marginally smaller, plus it is  
  a simple way to slightly obfuscate the code to thwart the casual observer.) All you
  need to do is drag CandyCrisis.exe onto the upx shortcut.
- Delete CandyCrisisResources\Preferences.txt and then run the game so that it is
  regenerated in a "virgin" state, with the default high scores and preferences set.
- Compile Inno.ins with Inno Setup Compiler.


Good luck! I think this source code can be used as a good example of how to convert an
old-school Mac game to work cross-platform using open-source libraries such as SDL.
The resources were all hand-converted to formats which could be read on any platform;
for example, PICTS became JPG/PNG, MOD resources became MOD files and 'snd ' resources
became WAV files.

Please, please don't judge my coding ability from this source! The original code was
largely written in my late-high-school/early-college years. My programming ability 
and style has matured and changed greatly since then. I think the final product came 
out really well, but I'll be the first to admit that this code uses many ugly tricks
and isn't a great example of how to write clean code. Its evolution from Mac OS 7, to 
OS X, and then to SDL and Windows, has also served to complicate the code a bit.


Questions? <johnstiles@warpmail.net>