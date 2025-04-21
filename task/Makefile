CC = g++ -fopenmp
PROG = task
SRCS = main.cpp CFileReader.cpp CSolver.cpp
OBJS = $(SRCS:.cpp=.o)
HDRS = CFileReader.h CSolver.h

all: clean $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $^

%.o: %.cpp $(HDRS)
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROG)
