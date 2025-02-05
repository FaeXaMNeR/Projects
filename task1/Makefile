CXX = gcc

CXXFLAGS = -Wall -g

TARGET = terminal

SRCS = main.c execcmd.c nums.c parse.c

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
