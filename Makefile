all: program run clean

program: editor.c libs/raylib/raylib.h
	gcc -o editor.exe editor.c libs/raylib/raylib.h libs/raygui.h libs/raylib/libraylib.a -Wall -lgdi32 -lwinmm -mwindows

run:
	editor.exe

clean:
	erase editor.exe
