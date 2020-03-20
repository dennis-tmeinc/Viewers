#MinGW makefile for PWViewer
#
#
#

#PREFIX=x86_64-w64-mingw32-
PREFIX=i686-w64-mingw32-
CC=$(PREFIX)gcc
WINDRES=$(PREFIX)windres
STRIP=$(PREFIX)strip

WINFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__  -D_WIN32_WINNT=0x0601
CFLAGS = $(WINFLAGS) -Os -mwindows
LFLAGS = -lmsvcrt -lgdiplus -lcomctl32 -lversion -lole32 -lurlmon -luuid -lnetapi32 -lwinmm -lws2_32 -loleaut32 -lsupc++

TARGET = PWViewer

SOURCES = $(TARGET).cpp \
        stdafx.cpp\
        ../common/CallTmeDlg.cpp \
        ../common/Screen.cpp \
        ../common/cliplist.cpp \
        ../common/Capture4.cpp \
        ../common/SelectDvr.cpp\
        ../common/crypt.cpp\
        ../common/util.cpp\
        ../common/decoder.cpp\
        ../common/volumebar.cpp\
        ../common/ChangePassword.cpp\
        ../common/SimpleBrowser.cpp\
        ../common/WebWin.cpp\
        ../common/json.cpp\
        ../common/SubViewScreen.cpp\
        ../common/dvrclientDlg.cpp\
        ../common/VideoPassword.cpp\
        ../common/CopyProgress.cpp\
        ../common/UserPass.cpp\
        ../common/md5.cpp\
        ../common/RemoteIpDlg.cpp\
        ../common/sliderbar.cpp \
        ../common/bsliderbar.cpp \
		../common/tvsid.cpp


HEADERS = $(TARGET).h \
            dvrclient.h \
            resource.h \
            ../common/cwin.h \
            ../common/cdir.h \
            ../common/cstr.h \
            ../common/util.h\
            ../common/CallTmeDlg.h\
            ../common/SelectDvr.h\
            ../common/map.h\
            ../common/Capture4.h\
            ../common/User.h\
            ../common/json.h\
            ../common/SubViewScreen.h\
            ../common/cliplist.h\
            ../common/UserNew.h\
            ../common/crypt.h\
            ../common/sliderbar.h\
            ../common/ChangePassword.h\
            ../common/UserPass.h\
            ../common/CopyProgress.h\
            ../common/RemoteIpDlg.h\
            ../common/Volumebar.h\
            ../common/decoder.h\
            ../common/Screen.h\
            ../common/bitmapbutton.h\
            ../common/dvrclientDlg.h \
			../common/tvsid.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
		
$(TARGET).exe : $(OBJS) $(TARGET)_res.o ../tmeid/tmeid.dll
	$(CC) $(CFLAGS) -o $@ $^  $(LFLAGS)

$(TARGET)_res.o : $(TARGET).rc res/$(TARGET).rc2
	$(PREFIX)windres $(WINFLAGS) $(TARGET).rc $@
	
%.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

strip : $(TARGET).exe
	$(STRIP) $^
	
clean :
	rm -f $(TARGET).exe $(OBJS) *.o
