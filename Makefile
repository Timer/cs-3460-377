COMPILER=g++-5
FLAGS=-std=c++11

EXECUTABLES=1-hello-world.out 2-prime-numbers.out 3-convolution.out 4-sort.out 5-static-serve.out

all:
	make $(EXECUTABLES)

1-hello-world.out: homework-1/1-hello-world.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
2-prime-numbers.out: homework-2/2-prime-numbers.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
3-convolution.out: homework-3/3-convolution.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
4-sort.out: homework-4/4-sort.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
5-static-serve.out: homework-5/5-static-serve.cpp
	$(COMPILER) $(FLAGS) -o $@ $^

clean:
	rm -f $(GARBAGE) $(EXECUTABLES)
