#MinGW makefile for gendvrid
#
#
#

CC=i686-w64-mingw32-gcc

WINFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__ -DWINVER=0x0501 -D_WIN32_IE=0x600
CFLAGS = $(WINFLAGS) -s -O2 -mwindows
LFLAGS = -lgdiplus -lcomctl32 -static-libstdc++ -static-libgcc

TARGET = gendvrid

SOURCES = gendvrid.cpp 

HEADERS = gendvrid.h \
            resource.h \
            ../common/cwin.h \
            ../common/cdir.h \
            ../common/cstr.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
		
$(TARGET).exe : $(OBJS) $(TARGET)_res.o
	$(CC) $(CFLAGS) -o $@ $^ ../Debug/tmeid.dll $(LFLAGS)
	cp $@ ../Debug/
	cp $@ ../Release/

$(TARGET)_res.o : $(TARGET).rc 
	windres $(WINFLAGS) $(TARGET).rc $@

%.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
	
clean :
	rm -f $(TARGET).exe $(OBJS) *.o
