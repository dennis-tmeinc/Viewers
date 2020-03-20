#MinGW makefile for PWManager
#
#
#
PREFIX=x86_64-w64-mingw32-
#PREFIX=i686-w64-mingw32-
CC=$(PREFIX)gcc
WINDRES=$(PREFIX)windres
STRIP=$(PREFIX)strip

CFLAGS =  -v # -I/usr/share/mingw-w64/include 

WINFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__ -DWINVER=0x0501 -D_WIN32_IE=0x600
CFLAGS += $(WINFLAGS) -O2 -mwindows
LFLAGS = -lgdiplus -lcomctl32 -lversion -lurlmon -lole32 -luuid -lnetapi32 -lwinmm -lws2_32 -loleaut32 -lsupc++

TARGET = PWManager

SOURCES = $(TARGET).cpp \
		  ManagerDlg.cpp \
		  ../common/util.cpp

HEADERS = PWManager.h \
		  ManagerDlg.h \
		  DlgClTargetProgress.h \
		  DlgCleanTarget.h \
		  DlgSrcId.h \
		  resource.h \
		  ../common/cwin.h \
		  ../common/cdir.h \
		  ../common/cstr.h \
		  ../common/util.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
		
$(TARGET).exe : $(OBJS) $(TARGET)_res.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)
	$(STRIP) $@

$(TARGET)_res.o : $(TARGET).rc
	$(WINDRES) $(WINFLAGS) $(TARGET).rc $@

%.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
	
../tmeid/tmeid.dll:
	$(MAKE) -C ../tmeid

clean :
	rm -f $(TARGET).exe $(OBJS) *.o