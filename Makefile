all: program run clean

program: editor.c
	gcc -o editor.exe editor.c -lraylib -lgdi32 -lwinmm -std=c99

run:
	editor.exe

clean:
	erase editor.exe
