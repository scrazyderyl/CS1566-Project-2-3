OS := $(shell uname)

ifeq ($(OS),Darwin)
	OPTIONS = -framework GLUT -framework OpenGL
	DEFINES = -D GL_SILENCE_DEPRECATION
else
	OPTIONS = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL
	DEFINES = 
endif

template: maze.c maze_algorithms.o initShader.o myLib.o
	gcc -o maze maze.c maze_algorithms.o initShader.o myLib.o $(OPTIONS) $(DEFINES)

maze_algorithms.o: maze_algorithms.c maze_algorithms.h
	gcc -c maze_algorithms.c $(DEFINES)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c $(DEFINES)

myLib.o: myLib.c myLib.h
	gcc -c myLib.c $(DEFINES)

clean:
	rm -f maze maze_algorithms.o initShader.o myLib.o