OPTIONS = -framework GLUT -framework OpenGL
DEFINES = -D GL_SILENCE_DEPRECATION

template: maze.c initShader.o myLib.o
	gcc -o maze maze.c initShader.o myLib.o $(OPTIONS) $(DEFINES)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c $(DEFINES)

tempLib.o: myLib.h myLib.c
	gcc -c myLib.c $(DEFINES)
