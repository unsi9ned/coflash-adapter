@echo off
set CXX=g++
set CXXFLAGS=-std=c++11 -Wall -O2
set TARGET=coflash.exe
set SRCDIR=src
set OBJDIR=obj

if not exist %OBJDIR% mkdir %OBJDIR%

echo Compiling coflash.cpp...
%CXX% %CXXFLAGS% -c %SRCDIR%\coflash.cpp -o %OBJDIR%\coflash.o
if %errorlevel% neq 0 goto error

echo Compiling pyocd_cmd_builder.cpp...
%CXX% %CXXFLAGS% -c %SRCDIR%\pyocd_cmd_builder.cpp -o %OBJDIR%\pyocd_cmd_builder.o
if %errorlevel% neq 0 goto error

echo Linking...
%CXX% %OBJDIR%\coflash.o %OBJDIR%\pyocd_cmd_builder.o -o %TARGET%
if %errorlevel% neq 0 goto error

echo Build successful: %TARGET%
goto end

:error
echo Build failed!
exit /b 1

:end