COMPILER=g++-5
FLAGS=-std=c++11

EXECUTABLES=1-hello-world.out 2-prime-numbers.out 3-convolution.out 4-sort.out 5-static-serve.out

all:
	make $(EXECUTABLES)

1-hello-world.out: 1-hello-world.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
2-prime-numbers.out: 2-prime-numbers.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
3-convolution.out: 3-convolution.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
4-sort.out: 4-sort.cpp
	$(COMPILER) $(FLAGS) -o $@ $^
5-static-serve.out: 5-static-serve.cpp
	$(COMPILER) $(FLAGS) -o $@ $^

clean:
	rm -f $(GARBAGE) $(EXECUTABLES)
