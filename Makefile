main: main.o view.o
	g++ -o main $^ -framework OpenGL

clean:
	rm -f main main.o view.o
