CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -L /usr/local/lib/ -lboost_system
TARGET_DIR = build
TARGET = $(TARGET_DIR)/test_connection

all:clean $(TARGET) run

$(TARGET): $(TARGET_DIR) main2.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main2.cpp $(LDFLAGS)

.PHONY: run clean

clean:
	rm -rf $(TARGET_DIR) clear

run: printout
	./$(TARGET)

printout:
	@echo "Starting program at"

$(TARGET_DIR):
	mkdir -p $@
