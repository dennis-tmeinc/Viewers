#MinGW makefile for TMEID
#
#	remove #include <stdafx.h> line if compiler failed!!!
#

#PREFIX=x86_64-w64-mingw32-
PREFIX=i686-w64-mingw32-
CC=$(PREFIX)gcc
WINDRES=$(PREFIX)windres

CPPFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__ -D_WIN32_WINNT=0x0601
CFLAGS = -Os -mwindows -shared -fno-exceptions
LDFLAGS = -lgdiplus -lcomctl32 -lversion -luserenv -lole32 -luuid -lnetapi32 -lwinmm -lws2_32 -loleaut32 -lsupc++

SOURCES = tmeid.cpp diskid.cpp md5.cpp
HEADERS = resource.h ../common/cstr.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

TARGET = tmeid

$(TARGET).dll : $(OBJS) $(TARGET)_res.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

$(TARGET)_res.o : $(TARGET).rc
	$(WINDRES) $(CPPFLAGS) $(TARGET).rc $@
	
%.o : %.cpp $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@	

clean :
	rm -f $(TARGET).dll $(OBJS) *.o
