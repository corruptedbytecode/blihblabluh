CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic
TARGET   = luaprint
SRCS     = main.cpp parser.cpp printer.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
