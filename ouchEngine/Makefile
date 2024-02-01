include ../Makefile.inc

CFLAGS = -c -Wall -O3 --std=c++11 -I$(OLTS_INC) -L$(OLTS_LIB) -L/usr/local/lib -L/usr/local/lib64

CPGM = miscfunc config TEOrderPackage GSocket journal mutexGuard CSemaphore RawPackage GPackage GFile AESRijndael minilzo CMCrypt AutoOrder TrailingOrder PriceFraction etf find_itch ETFOrder BasketOrder SoupBin Itch Ouch IPackage InlineOrder ItchMap utils TSession StockInfo OrderBookDirectory ClientConverter OuchEngine \

all: $(CPGM)
#	$(CC) $(CFLAGS) $@.cc

aes:
	cd aes
	make all

$(CPGM) : $@
	$(CC) $(CFLAGS) $@.cc

lib : $(CPGM)
	cd aes
	make all
	ar crv libmisc.a *.o aes/*.o
	ranlib libmisc.a
	mv libmisc.a ../../libs/
	rm *.o
#	rm aes/*.o

clean :
	rm *.o
	rm aes/*.o
