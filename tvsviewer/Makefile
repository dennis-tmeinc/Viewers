#MinGW makefile for tvsviewer
#
#
#

#PREFIX=x86_64-w64-mingw32-
PREFIX=i686-w64-mingw32-
CC=$(PREFIX)gcc
WINDRES=$(PREFIX)windres
STRIP=$(PREFIX)strip

CPPFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__ -D_WIN32_WINNT=0x0601
CFLAGS = -Os -mwindows
LDFLAGS = -lgdiplus -lcomctl32 -lversion -lole32 -lurlmon -luuid -lnetapi32 -lwinmm -lws2_32 -loleaut32 -lsupc++ 

TARGET = tvsviewer

SOURCES = $(TARGET).cpp \
    ../common/bsliderbar.cpp\
    ../common/Capture4.cpp\
    ../common/json.cpp\
    ../common/cliplist.cpp\
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
    ../common/SubViewScreen.cpp\
    ../common/WebWin.cpp

HEADERS = $(TARGET).h \
    ../common/CallTmeDlg.h\
    ../common/Capture4.h\
    ../common/json.h\
    ../common/cdir.h\
    ../common/ChangePassword.h\
    ../common/cliplist.h\
    ../common/CopyProgress.h\
    ../common/crypt.h\
    ../common/cstr.h\
    ../common/cwin.h\
    ../common/decoder.h\
    ../common/dvrclientDlg.h\
    ../common/RemoteIpDlg.h\
    ../common/Screen.h\
    ../common/SelectDvr.h\
    ../common/sliderbar.h\
    ../common/tvsid.h\
    ../common/User.h\
    ../common/UserNew.h\
    ../common/UserPass.h\
    ../common/util.h\
    ../common/VideoPassword.h\
    ../common/Volumebar.h\
    ../common/SubViewScreen.h\
    dvrclient.h\
    resource.h\
    targetver.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
		
$(TARGET).exe : $(OBJS) $(TARGET)_res.o ../tmeid/tmeid.dll
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

$(TARGET)_res.o : $(TARGET).rc res/$(TARGET).rc2
	$(PREFIX)windres $(CPPFLAGS) $(TARGET).rc $@

%.o : %.cpp $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

strip : $(TARGET).exe
	$(STRIP) $^

clean :
	rm -f $(TARGET).exe $(OBJS) *.o
