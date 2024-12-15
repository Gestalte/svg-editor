program: editor.c libs/raylib/raylib.h
	gcc -o editor.exe editor.c libs/raylib/raylib.h libs/raylib/libraylib.a -Wall -lgdi32 -lwinmm -mwindows
