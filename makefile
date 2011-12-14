all: htm

htm: htm.c
	gcc -g $^ -o $@ -lc -lGL -lGLU -lglut
