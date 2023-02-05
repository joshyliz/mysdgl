all:
	gcc src/*.c -I include/ -L lib/ -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lfreetype -ldl -o sdgl.exe
	./sdgl.exe
