all: program run clean

program: editor.c
	gcc -o editor.exe editor.c -lraylib -Wall -lgdi32 -lwinmm -mwindows

run:
	editor.exe

clean:
	erase editor.exe
