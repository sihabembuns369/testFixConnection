CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -L /usr/local/lib/ 
LDFLAGS_DEBUG =  $(LDFLAGS) -g -lboost_system -lboost_thread
TARGET_DIR = ./build
TARGET = $(TARGET_DIR)/test

all: $(TARGET)

$(TARGET): $(TARGET_DIR) ./src/FixClient.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) ./src/FixClient.cpp $(LDFLAGS)

.PHONY: run clean

clean:
	rm -rf $(TARGET_DIR)

run: printout
	./$(TARGET)

printout:
	@echo "Starting program at"

$(TARGET_DIR):
	mkdir -p $@
