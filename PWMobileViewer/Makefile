#MinGW makefile for PWMobileViewer
#
#
#

WINFLAGS = -I. -I../common -D_UNICODE -DUNICODE -D__MINGW__ -DWINVER=0x0501 -D_WIN32_IE=0x600
CFLAGS = $(WINFLAGS) -s -O2 -mwindows
LFLAGS = -lgdiplus -lcomctl32 -lversion -lole32 -luuid -lnetapi32 -lwinmm -lws2_32 -loleaut32 -static-libstdc++ -static-libgcc

TARGET = PWMobileViewer

SOURCES = PWMobileViewer.cpp \
        ../common/CallTmeDlg.cpp \
        ../common/Screen.cpp \
        ../common/cliplist.cpp \
        ../common/tvsid.cpp \
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
        ../common/VideoPassword.cpp\
        ../common/sliderbar.cpp

HEADERS = PWMobileViewer.h \
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
            ../common/tvsid.h\
            ../common/CopyProgress.h\
            ../common/VideoPassword.h\
            ../common/RemoteIpDlg.h\
            ../common/Volumebar.h\
            ../common/decoder.h\
            ../common/Screen.h\
            ../common/bitmapbutton.h\
            ../common/dvrclientDlg.h

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
		
$(TARGET).exe : $(OBJS) $(TARGET)_res.o
	g++ $(CFLAGS) -o $@ $^ ../Debug/tmeid.dll /c/windows/system32/urlmon.dll $(LFLAGS)
	cp $@ ../Debug/
	cp $@ ../Release/

$(TARGET)_res.o : $(TARGET).rc
	windres $(WINFLAGS) $(TARGET).rc $@

%.o : %.cpp $(HEADERS)
	g++ $(CFLAGS) -c $< -o $@
	
clean :
	rm -f $(TARGET).exe $(OBJS) *.o
