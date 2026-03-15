CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -fno-exceptions -fno-rtti
TARGET = riscv_sim

SRCS = riscv_core.cpp parser.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
