# Better Desktop Sorting

This is a windows utility that sorts the desktop in differently than the default.

The included modes are:
- Purely Alphabetical (A)
- By file-type, with folders first, then shortcuts, then the rest (F)
- By file-type, but with shortcuts on the right (D)

To see their effects, look in the Images/ folder
## Installation

First of all, Download the [latest release](https://github.com/Xavlume/BetterDesktopSorting/releases/latest) and drag "BetterDesktopSorting.exe" anywhere on your computer. 

Then, run it once with icons in all four corners of the destop and four icons in the top left, as shown in the image.
![Setup_Image](Images/Setup.png)

Afterwards, run it again to sort your desktop!
## Configuration

Go to your appdata folder by pressing Windows+r at the same time to open the "run" application and enter %appdata% in the field.

Then, locate the "BetterDesktopSorting: folder and open it.

Locate the "Config.txt" file in the folder and open it with your preferred text editor. 

To change the sorting method, change the last line of the file with the character corresponding with the desired sorting method, the default being F, or file-type. The different sorting methods are listed in the introduction.
![Config_Image](Images/Config.png)
## TO-DO List
- Error Handling
- Automatic configuration
- Customisable file-type sorting
