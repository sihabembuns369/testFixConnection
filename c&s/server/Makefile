CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -L /usr/local/lib/ -lboost_system
TARGET_DIR = ../build/server
TARGET = $(TARGET_DIR)/server

all:clean $(TARGET) run

$(TARGET): $(TARGET_DIR) server.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) server.cpp $(LDFLAGS)

.PHONY: run clean

clean:
	rm -rf $(TARGET_DIR) clear

run: printout
	./$(TARGET)

printout:
	@echo "Starting program at"

$(TARGET_DIR):
	mkdir -p $@
