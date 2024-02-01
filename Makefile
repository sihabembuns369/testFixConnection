CXX = g++
CXXFLAGS = -Wall -O3 -std=c++0x -g  -fexceptions -Wno-deprecated-declarations -m64  -Wdeprecated-declarations `xml2-config --cflags` -fPIC   -lboost_date_time  -lboost_regex -DTERMINATE_IN_STD -DTYPEINFO_IN_STD -lzmq -lm -lmcrypt 
LDFLAGS =  -I/usr/local/includes  -L /usr/local/lib/ -lboost_system  -s -pthread -lboost_system -lboost_thread -ltbb `xml2-config --libs` -lxml2  -lboost_date_time -lpthread  -lboost_regex -lm -lzmq  -lmcrypt 
TARGET_DIR = build
TARGET = $(TARGET_DIR)/test_connection
SOURCES = test.cpp ./filesmain/*.cc ./filesmain/*.cpp ./ouchEngine/TEOrderPackage.cc ./ouchEngine/RawPackage.cc ./ouchEngine/CMCrypt.cc ./ouchEngine/GQueue.cc ./ouchEngine/Itch.cc ./ouchEngine/SoupBin.cc 
all:clean $(TARGET) run

$(TARGET): $(TARGET_DIR) $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

.PHONY: run clean

clean:
	rm -f *.o  # Menghapus file objek
	rm -rf $(TARGET_DIR)
	

run: printout
	clear
	./$(TARGET) ./conf/read.cnf ./DataDictionary/FIX50SP1-IDX.xml

printout:
	@echo "Starting program at"

$(TARGET_DIR):
	mkdir -p $@
