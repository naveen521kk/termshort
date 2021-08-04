screenshotter.exe: main.c
	gcc -g $? -o $@ -lgdi32 -luser32

format: main.c
	clang-format -i $?