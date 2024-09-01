CXX = g++
CFLAGS = -std=c++17 -O2 -Wall -g
TARGET = server
OBJS = ../http/*.cpp ../buffer/*.cpp ../server/*.cpp ../threadpool/*.cpp ../main.cpp 

all : $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET) -pthread

clean:
	rm -rf ../bin/$(TARGET)