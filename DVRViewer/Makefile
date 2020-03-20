#MinGW makefile for DVRViewer
#
#
#

#PREFIX=x86_64-w64-mingw32-
PREFIX=i686-w64-mingw32-
CC=$(PREFIX)gcc
WINDRES=$(PREFIX)windres
STRIP=$(PREFIX)strip

CPPFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__ -D_WIN32_WINNT=0x0601
CFLAGS = -O2 -mwindows 
LDFLAGS = -lgdiplus -lcomctl32 -lversion -lole32 -lurlmon -luuid -lnetapi32 -lwininet -lwinmm -lws2_32 -loleaut32 -lsupc++ ../pdf/libhpdf.dll

TARGET = DVRViewer

SOURCES = DVRViewer.cpp \
        ../common/CallTmeDlg.cpp \
        ../common/Screen.cpp \
        ../common/SubViewScreen.cpp\
        ../common/cliplist.cpp \
        ../common/Capture.cpp \
        ../common/SelectDvr.cpp\
        ../common/crypt.cpp\
        ../common/util.cpp\
        ../common/User.cpp\
        ../common/decoder.cpp\
        ../common/volumebar.cpp\
        ../common/ChangePassword.cpp\
        ../common/UserNew.cpp\
        ../common/dvrclientDlg.cpp\
        ../common/CopyProgress.cpp\
        ../common/UserPass.cpp\
        ../common/md5.cpp\
        ../common/RemoteIpDlg.cpp\
        ../common/sliderbar.cpp\
        ../common/SelectCamera.cpp\
        ../common/SimpleBrowser.cpp\
        ../common/WebWin.cpp\
        ../common/json.cpp\
		../common/DriveByReport.cpp

HEADERS = DVRViewer.h \
        dvrclient.h \
        resource.h \
        ../common/cwin.h \
        ../common/cdir.h \
        ../common/cstr.h \
        ../common/util.h\
        ../common/CallTmeDlg.h\
        ../common/SelectDvr.h\
        ../common/map.h\
        ../common/Capture.h\
        ../common/User.h\
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
        ../common/SubViewScreen.h\
        ../common/bitmapbutton.h\
        ../common/SelectCamera.h\
        ../common/WebWin.h\
		../common/json.h\
		../common/DriveByReport.h\
        ../common/dvrclientDlg.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
		
$(TARGET).exe : $(OBJS) $(TARGET)_res.o ../tmeid/tmeid.dll
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

strip : $(TARGET).exe
	$(STRIP) $^

../tmeid/tmeid.dll :
	$(MAKE) -C ../tmeid

$(TARGET)_res.o : $(TARGET).rc res/$(TARGET).rc2
	$(PREFIX)windres $(CPPFLAGS) $(TARGET).rc $@
	
%.o : %.cpp $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	
clean :
	rm -f $(TARGET).exe $(OBJS) *.o
