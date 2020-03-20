#MinGW makefile for pwplayer
#
#
#

#PREFIX=x86_64-w64-mingw32-
PREFIX=i686-w64-mingw32-
CC=$(PREFIX)gcc
WINDRES=$(PREFIX)windres
STRIP=$(PREFIX)strip

CPPFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__  -D_WIN32_WINNT=0x0601
CFLAGS = $(CPPFLAGS) -Os -mwindows
LDFLAGS = -static-libgcc -lmsvcrt -lgdiplus -lurlmon -lcomctl32 -lversion -lole32 -luuid -lnetapi32 -lwinmm -lws2_32 -loleaut32 -lsupc++

TARGET = pwplayer

SOURCES = \
    ../common/bsliderbar.cpp\
    ../common/CallTmeDlg.cpp\
    ../common/Capture4.cpp\
    ../common/ChangePassword.cpp\
    ../common/cJSON.cpp\
    ../common/cliplist.cpp\
    ../common/CopyProgress.cpp\
    ../common/crypt.cpp\
    ../common/decoder.cpp\
    ../common/dvrclientDlg.cpp\
    ../common/md5.cpp\
    ../common/RemoteIpDlg.cpp\
    ../common/Screen.cpp\
    ../common/SelectDvr.cpp\
    ../common/SimpleBrowser.cpp\
    ../common/sliderbar.cpp\
    ../common/tvsid.cpp\
    ../common/util.cpp\
    ../common/VideoPassword.cpp\
    ../common/volumebar.cpp\
    ../common/WebWin.cpp\
    pwplayer.cpp

HEADERS1 = \
    ../common/bsliderbar.h\
    ../common/CallTmeDlg.h\
    ../common/Capture4.h\
    ../common/ChangePassword.h\
    ../common/cJSON.h\
    ../common/cliplist.h\
    ../common/CopyProgress.h\
    ../common/crypt.h\
    ../common/decoder.h\
    ../common/dvrclientDlg.h\
    ../common/RemoteIpDlg.h\
    ../common/Screen.h\
    ../common/SelectDvr.h\
    ../common/tvsid.h\
    ../common/VideoPassword.h\
    ../common/Volumebar.h\
    ../common/WebWin.h\
    ../common/cwin.h \
    ../common/cdir.h \
    ../common/cstr.h \
    ../common/util.h\
    dvrclient.h\
    resource.h\
    pwplayer.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
		
$(TARGET).exe : $(OBJS) $(TARGET)_res.o ../tmeid/tmeid.dll
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

../tmeid/tmeid.dll :
	$(MAKE) -C ../tmeid

$(TARGET)_res.o : $(TARGET).rc res/$(TARGET).rc2
	$(PREFIX)windres $(WINFLAGS) $(TARGET).rc $@

%.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
	
clean :
	rm -f $(TARGET).exe $(OBJS) *.o
