CXX = g++
CFLAG = -g -Wall

all: game guess

%.o: %.cpp
	$(CXX) -c $< $(CFLAG)

game: game.o
	$(CXX) -o $@ $^

guess: guess.o
	$(CXX) -o $@ $^

clean:
	rm -f game game.o guess guess.o

.PHONY: all clean
