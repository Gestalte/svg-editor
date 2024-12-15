CC = gcc
NAME = editor
SHELL = CMD
DEPS = nanosvg.h nanosvgrast.h raylib.h raygui.h

all: compile run

compile: $(NAME).c $(DEPS)
	$(CC) -o $(NAME).exe $(NAME).c $(DEPS)

run:
	$(NAME).exe

