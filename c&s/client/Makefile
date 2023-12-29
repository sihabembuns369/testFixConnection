CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -L /usr/local/lib/ -lboost_system
TARGET_DIR = build/client
TARGET = $(TARGET_DIR)/client

all:clean $(TARGET) run

$(TARGET): $(TARGET_DIR) client.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) client.cpp $(LDFLAGS)

.PHONY: run clean

clean:
	rm -rf $(TARGET_DIR) clear

run: printout
	./$(TARGET)

printout:
	@echo "Starting program at"

$(TARGET_DIR):
	mkdir -p $@
