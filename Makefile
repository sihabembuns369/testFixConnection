CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wdeprecated-declarations
LDFLAGS = -L /usr/local/lib/ -lboost_system  -s -pthread -lboost_system -lboost_thread
TARGET_DIR = build
TARGET = $(TARGET_DIR)/test_connection

all:clean $(TARGET) run

$(TARGET): $(TARGET_DIR) test.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) test.cpp $(LDFLAGS)

.PHONY: run clean

clean:
	rm -rf $(TARGET_DIR) clear

run: printout
	./$(TARGET)

printout:
	@echo "Starting program at"

$(TARGET_DIR):
	mkdir -p $@
