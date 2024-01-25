CXX = g++
CXXFLAGS = -std=c++0x -g -Wall -fexceptions -Wno-deprecated-declarations -m64  -Wdeprecated-declarations `xml2-config --cflags` -fPIC   -lboost_date_time  -lboost_regex -DTERMINATE_IN_STD -DTYPEINFO_IN_STD
LDFLAGS = -L /usr/local/lib/ -lboost_system  -s -pthread -lboost_system -lboost_thread -ltbb `xml2-config --libs` -lxml2  -lboost_date_time -lpthread  -lboost_regex 
TARGET_DIR = build
TARGET = $(TARGET_DIR)/test_connection
SOURCES = test.cpp ./filesmain/*.cc ./filesmain/*.cpp 
all:clean $(TARGET) run

$(TARGET): $(TARGET_DIR) $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

.PHONY: run clean

clean:
	rm -rf $(TARGET_DIR)
	

run: printout
	clear
	./$(TARGET) ./conf/read.cnf ./DataDictionary/FIX50SP1-IDX.xml

printout:
	@echo "Starting program at"

$(TARGET_DIR):
	mkdir -p $@
