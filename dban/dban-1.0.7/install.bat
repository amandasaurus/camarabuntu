@echo off

echo.
echo  Install Darik's Boot and Nuke
echo  ------------------------------
echo.
echo  Please note that DBAN is very sensitive to media quality. If you use
echo  anything but a perfectly good floppy disk, then DBAN may not start.
echo.
echo  Remember to remove and properly label the DBAN floppy disk after it is
echo  written. You might reboot and accidentally wipe this computer by mistake.
echo.
echo  Product updates and additional documentation are available at:
echo  http://dban.sourceforge.net/
echo.
echo  Insert a blank floppy disk now and press any key to continue.
echo  Click "ignore" if a virus scanner traps access to the floppy drive.
echo.

pause > nul

REM Rawrite2 only takes 8.3 file names and RawWriteWin is buggy.
REM Copy the floppy image file to the temp directory so that we
REM can use a wildcard expansion instead of a short tilde name.

copy /b dban*.img %TEMP%\dban.img > nul

REM Write the floppy image to the first floppy drive.

rawrite2.exe -f %TEMP%\dban.img -d A: -n
echo Press any key or click the 'X' button to close this window.

REM Clean up and pause so that the user can see the result.

del %TEMP%\dban.img > nul
pause > nul

REM Clear the screen so that 95/98/ME systems close the window.

cls

REM eof
