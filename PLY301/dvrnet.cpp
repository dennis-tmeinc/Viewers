
// dvrnet.cpp
// DVR 301 net protocol
#include "stdafx.h"
#include "player.h"
#include "dvrnet.h"

#define TVSVIEWER

USHORT g_dvr_port = PORT_TMEDVR;
int g_lan_detect ;
int g_net_watchdog ;
int g_remote_support ;

// convert computer name (string) to sockad ( cp name example: 192.168.42.227 or 192.168.42.227:15111 )
// return 0 for success
int net_sockaddr(struct sockad * sad, const char * cpname, int port)
{
	int result = -1 ;
	char ipaddr[256] ;
	char servname[128] ;
	struct addrinfo * res ;
	struct addrinfo hint ;
	char * portonname ;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = PF_INET ;
	hint.ai_socktype = SOCK_STREAM ;
	strncpy( ipaddr, cpname, 255 );
	ipaddr[255]=0 ;
	portonname = strchr(ipaddr, ':');
	if( portonname ) {
		strncpy( servname, portonname+1, 120 );
		*portonname=0 ;
	}
	else {
		sprintf(servname, "%d", port);
	}
	if( getaddrinfo( ipaddr, servname, &hint, &res )==0 ) {
		if( res->ai_socktype == SOCK_STREAM ) {
			sad->len = res->ai_addrlen ;
			memcpy(&(sad->addr), res->ai_addr, sad->len );
			result = 0 ;
		}
		freeaddrinfo(res);
	}
	return result ;
}

// convert sockad to computer name
// return 0 for success
int net_sockname(char * cpname, struct sockad * sad)
{
	return getnameinfo( &(sad->addr), sad->len, cpname, 128, NULL, 0, NI_NUMERICHOST);
}

// return 1 if socket ready to read, 0 for time out
int net_wait(SOCKET sock, int timeout)
{
	struct timeval tout ;
	fd_set set ;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	tout.tv_sec=timeout/1000000 ;
	tout.tv_usec=timeout%1000000 ;
	return (select( (int)sock+1, &set, NULL, NULL, &tout))>0 ;
}

// clear received buffer
void net_clear(SOCKET sock)
{
	int r;
    char dummybuf[100] ;
	while( net_wait(sock, 1) ) {
		r=recv(sock, dummybuf, sizeof(dummybuf), 0);
		if( r<=0 ) {
			break;
		}
	}
}

// try to force server flush out data
void net_pingdata(SOCKET so)
{
/*
	char echobuf[100] ;
	struct Req_type req ;
	req.reqcode = REQECHO ;
	req.data = 0 ;
	req.reqsize = sizeof(echobuf);
	net_send( so, &req, sizeof(req) );
	net_send( so, echobuf, req.reqsize );
*/
}

// Shutdown socket
int net_shutdown( SOCKET s )
{
	return shutdown( s, SD_BOTH );
}

int serror ;

void net_send( SOCKET sock, void * buf, int bufsize )
{
	int slen ;
	char * cbuf=(char *)buf ;
	while( bufsize>0 ) {
		slen=send(sock, cbuf, bufsize, 0);
		if( slen<0 ) {
            int nError=WSAGetLastError();
            if( nError!=WSAEWOULDBLOCK) {
                return ;
            }
        }
        else {
            bufsize-=slen ;
            cbuf+=slen ;
        }
	}
}

// recv network data until buffer filled. 
// return 1: success
//		  0: failed
int net_recv( SOCKET sock, void * buf, int bufsize, int wait )
{
	int retry=3 ;
	int ping=0;
	int recvlen ;
	int cbufsize=bufsize;
	char * cbuf = (char *) buf ;
	retry = wait/100000 ;
	while( cbufsize>0 ) {
		if( net_wait(sock, 100000)>0 ) {			// data available? Timeout 3 seconds
			recvlen=recv(sock, cbuf, cbufsize, 0 );
			if( recvlen>0 ) {
				cbufsize-=recvlen ;
				cbuf+=recvlen ;
			}
            else if ( recvlen==0 ) {
                // socket closed
                return 0 ;
            }
			else {
                int nError=WSAGetLastError();
                if( nError!=WSAEWOULDBLOCK ) {
                    return 0 ;      // error!
                }
            }
		}
		else {
			if( retry-->0 ) {
				net_pingdata(sock);
				ping++;
			}
			else {
				return 0;									// timeout or error
			}
		}
	}
	return 1;						// success
}

static DWORD net_finddevice_tick ;
static SOCKET net_find_sock = INVALID_SOCKET ;

// finding dvr device by broadcasting
int net_finddevice()
{
    INTERFACE_INFO InterfaceList[20];
    unsigned long nBytesReturned;
	DWORD req=REQDVREX ;
	struct sockad mcaddr ;		// multicase address
	int i;

	net_finddevice_tick = GetTickCount();

	if( net_find_sock == INVALID_SOCKET ) {
		net_find_sock=socket(AF_INET, SOCK_DGRAM, 0);
	}

	// send multicast dvr search requests (to our dvr mc adress:228.229.230.231)
	net_sockaddr( &mcaddr, "228.229.230.231", g_dvr_port);
	DWORD ttl=30;
	setsockopt(net_find_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
	sendto(net_find_sock, (char *)&req, sizeof(req),0, &mcaddr.addr, mcaddr.len );

    if (WSAIoctl(net_find_sock, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
		sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) {
		return 0;
    }

	// sending broadcast dvr search requests
    int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
    for ( i = 0; i < nNumInterfaces; ++i) {
		if( (InterfaceList[i].iiFlags & IFF_BROADCAST) &&				// can bcast
			(InterfaceList[i].iiFlags & IFF_UP) &&						// interface up
			(InterfaceList[i].iiAddress.Address.sa_family==AF_INET) )
		{				
			InterfaceList[i].iiAddress.AddressIn.sin_addr.S_un.S_addr |=
					~(InterfaceList[i].iiNetmask.AddressIn.sin_addr.S_un.S_addr) ;
			InterfaceList[i].iiAddress.AddressIn.sin_port=htons(g_dvr_port);
			sendto(net_find_sock, (char *)&req, sizeof(req),0, &InterfaceList[i].iiAddress.Address, sizeof(InterfaceList[i].iiAddress.AddressIn));
		}
    }

    return 	0 ;

}

// detecting device through ip address
int net_detectdevice( char * ipaddr )
{
	DWORD req=REQDVREX ;
	struct sockad saddr ;

	DWORD ttl=30;

	if( net_find_sock != INVALID_SOCKET ) {
		if( net_sockaddr(&saddr, ipaddr, g_dvr_port)==0 ) {
			sendto(net_find_sock, (char *)&req, sizeof(req),0, &saddr.addr, saddr.len );
		}
		net_finddevice_tick = GetTickCount();
	}

	return 	0 ;
}

int net_polldevice()
{
	struct sockad recvaddr ;
	int recvlen ;
	char buf[128] ;
	int i;

	// receiving DVR responses
	while( net_wait(net_find_sock, 0)>0 ) {
		recvaddr.len = sizeof(recvaddr)-sizeof(int);
		recvlen=recvfrom(net_find_sock,  buf, sizeof(buf),  0,  &(recvaddr.addr), &(recvaddr.len) );
		if( recvlen>=sizeof(DWORD) ) {
			if( *(DWORD *)buf==DVRSVREX ) {
				net_finddevice_tick = GetTickCount();
				if( net_sockname(buf, &recvaddr)==0 ) {
					for( i=0; i<g_dvr_number; i++ ) {
						if( strcmp( g_dvr[i].name, buf )==0 ) {
							break ;
						}
					}
					if( i==g_dvr_number && g_dvr_number<MAX_DVR_DEVICE ) {
                        strcpy( g_dvr[g_dvr_number].name, buf );
						g_dvr[g_dvr_number].type = DVR_DEVICE_TYPE_DVR ;
						g_dvr_number++ ;
					}
				}
			}
		}
	}
	if( (int)(GetTickCount()-net_finddevice_tick) > 5000 ) {
		closesocket( net_find_sock ) ;
		net_find_sock = INVALID_SOCKET ;
		return 1 ;
	}
	else {
		return 0 ;
	}
}

// connect to a DVR server
SOCKET net_connect( char * dvrserver, int port )
{
	SOCKET so ;
	struct sockad soad ;
    if( port<=0 ) {
        port = g_dvr_port ;
    }
	if( net_sockaddr( &soad, dvrserver, port )==0 ) {
		so=socket(AF_INET, SOCK_STREAM, 0);
		if( connect(so, &(soad.addr), soad.len )==0 ) {
			BOOL nodelay=1 ;
			setsockopt( so, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay) );
//            u_long nblock=1 ;
//            ioctlsocket(so,FIONBIO,&nblock); 
			return so ;
		}
		closesocket(so);
	}
	return 0 ;
}

// close a connection
void net_close(SOCKET so)
{
	closesocket(so);
}

// get server(player) information. 
//        setup ppi->servername, ppi->total_channel, ppi->serverversion
// return 1: success
//        0: failed
int net_getserverinfo( SOCKET so, struct player_info * ppi )
{
	struct Req_type req ;
	struct Answer_type ans ;
	struct sockad sad ;

	int res=0 ;
	char * pbuf ;

	if( ppi->size<sizeof( struct player_info ) )
		return res ;

	net_clear(so);		// clear received buffer

	// Get DVR server IP name
	memset( &sad, 0, sizeof(sad));
	sad.len = sizeof(sad)-sizeof(sad.len) ;
	getpeername(so, &sad.addr, &sad.len);
	net_sockname(ppi->serverinfo, &sad);
	strncpy(ppi->servername, ppi->serverinfo, sizeof(ppi->servername));	// default server name

/*
	// get server name
	req.reqcode = REQSERVERNAME ;
	req.data = 0 ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req));
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSERVERNAME && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				pbuf[ans.anssize-1]='\0' ;
				strncpy(ppi->servername, pbuf, sizeof(ppi->servername));
				res=1 ;
			}
			delete pbuf ;
		}
	}
*/
	struct system_stru sys ;
	if( net_GetSystemSetup(so, &sys, sizeof(sys)) ) {
		memcpy(&(ppi->serverinfo[48]), sys.productid, 80 ) ;
		strncpy(ppi->servername, sys.ServerName, sizeof(ppi->servername));
		// check PTZ support
		if( sys.ptz_en ) {
			ppi->features |= PLYFEATURE_PTZ ;
		}
		ppi->total_channel = sys.cameranum ;
		res=1 ;
	}

	// get DVR version
	req.reqcode = REQGETVERSION ;
	req.data = 0 ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETVERSION && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize >= 4*sizeof(int) ) {
					memcpy( ppi->serverversion, pbuf, 4*sizeof(int));
					res=1 ;
				}
			}
			delete pbuf ;
		}
	}

/*
	// get channel number
	ppi->total_channel = 0 ;

	req.reqcode = REQCHANNELINFO ;
	req.data = 0 ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req));

	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSCHANNELDATA ) {
			ppi->total_channel = ans.data ;
			res=1 ;
		}
		if( ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			pbuf = new char [ans.anssize] ;
			net_recv(so, pbuf, ans.anssize );
			delete pbuf ;
		}
	}
*/

	return res;
}

// return number of DVR video channels
// return 0: error, 1: success
int net_channelinfo( SOCKET so, struct channel_info * pci, int totalchannel)
{
	int res=0;
	int i;

	net_clear(so);		// clear received buffer

	DvrChannel_attr	cam_attr ;
	for( i=0; i<totalchannel ; i++ ) {
		memset(&cam_attr, 0, sizeof( cam_attr ) );
		if( net_GetChannelSetup(so, &cam_attr, sizeof( cam_attr ), i ) ) {
			strncpy( pci[i].cameraname, cam_attr.CameraName, sizeof( pci[i].cameraname ) );
			if( cam_attr.Enable ) {
				pci[i].features |= 1 ;			// enabled bit
			}
			if( cam_attr.ptz_addr >= 0 ) {
				pci[i].features |= 2 ;
			}
			pci[i].status = 0 ;
			if( cam_attr.Resolution==0 ) {		// cif
				pci[i].xres = 352 ;
				pci[i].yres = 240 ;
			}
			else if( cam_attr.Resolution==1 ) {	// 2cif
				pci[i].xres = 704 ;
				pci[i].yres = 240 ;
			}
			else if( cam_attr.Resolution==2 ) {	// dcif
				pci[i].xres = 528 ;
				pci[i].yres = 320 ;
			}
			else if( cam_attr.Resolution==3 ) {	// 4cif
				pci[i].xres = 704 ;
				pci[i].yres = 480 ;
			}
			res=1 ;
		}
	}

/*

	struct dvr_channel_info {
		int	Enable ;
		int Resolution ;
		char CameraName[64] ;
	} * pdvrchinfo;

	struct Req_type req ;
	struct Answer_type ans ;
	char * pbuf ;					// receiving buffer

	int n, i;
	int res=0 ;

	if( pci->size<sizeof( struct channel_info ) )
		return res ;

	req.reqcode = REQCHANNELINFO ;
	req.data = totalchannel ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req));

	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSCHANNELDATA && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				n=ans.anssize/sizeof(struct dvr_channel_info) ;
				if( n>totalchannel ) 
					n=totalchannel ;
				if( n>0 ) {
					pdvrchinfo = (struct dvr_channel_info *)pbuf ;
					for( i=0; i<n; i++ ) {
						strcpy( pci[i].cameraname, pdvrchinfo[i].CameraName);
						pci[i].features = 0 ;
						if( pdvrchinfo[i].Enable ){
							pci[i].features |= 1 ;			// enabled bit
						}
						pci[i].status = 0 ;
					}
					res=1 ;
				}
			}
			delete pbuf ;
		}
	}
*/

/*  skip status bits...

	struct dvr_channelstatus {
		int sig;
		int rec;
		int mot;
	} * pdvrchst ;

	// get channel status 
	req.reqcode = REQGETCHANNELSTATE ;
	req.data = totalchannel ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req));

	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETCHANNELSTATE && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				n=ans.anssize/sizeof(struct dvr_channelstatus) ;
				if( n>totalchannel ) 
					n=totalchannel ;
				if( n>0 ) {
					pdvrchst = (struct dvr_channelstatus *)pbuf ;
					for(i=0;i<n;i++) {
						if( pdvrchst[i].sig )
							pci[i].status |= 1 ;			// bit 0: signal lost
						if( pdvrchst[i].rec ) 
							pci[i].status |= 4 ;			// bit 2: recording
						if( pdvrchst[i].mot ) 
							pci[i].status |= 2 ;			// bit 1: motion detected
					}
					res = 1 ;
				}
			}
			delete pbuf ;
		}
	}

*/

	return res ;
}


// Get DVR server version
// parameter
//		so: socket
//		version: 4 int array 
// return 0: error, 1: success
int net_dvrversion(SOCKET so, int * version)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;
	net_clear(so);		// clear received buffer
	req.reqcode = REQGETVERSION ;
	req.data = 0 ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETVERSION && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize >= 4*sizeof(int) ) {
					memcpy( version, pbuf, 4*sizeof(int));
					res=1 ;
				}
			}
			delete pbuf ;
		}
	}
	return res;
}

// Start play a preview channel
// return 1 : success
//        0 : failed
int net_preview_play( SOCKET so, int channel )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;
	net_clear(so);		// clear received buffer
	req.data=channel ;
	req.reqcode=REQREALTIME;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSREALTIMEHEADER && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( pbuf[0]== '4' &&
					pbuf[1]== 'H' && 
					pbuf[2]== 'K' && 
					pbuf[3]== 'H' ){									// check hikvision H.264 header
					res=1 ;
				}
			}
			delete pbuf ;
		}
	}
	return res ;
}

// Get data from preview stream
// return 1 : success
//        0 : failed
int net_preview_getdata( SOCKET so, char ** framedata, int * framesize )
{
	if( net_wait(so, RECV_TIMEOUT)==1 ) {
		* framedata = new char [102400] ;
		* framesize = recv(so, *framedata, 102400, 0) ;
		if( *framesize<=0 ) {
			delete * framedata ;
			* framedata = NULL ;
			* framesize = 0 ;
			return 0 ;			// failed 
		}
		return 1 ;
	}
	return 0 ;
}

// open a live stream
// return 0 : failed
//        1 : success
int net_live_open( SOCKET so, int channel )
{
	struct Req_type req ;
	struct Answer_type ans ;
	net_clear(so);		// clear received buffer
	req.data=channel ;
	req.reqcode=REQOPENLIVE;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSTREAMOPEN ) {
			if( ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
				char * pbuf = new char [ans.anssize] ;
				net_recv(so, pbuf,ans.anssize);
				delete pbuf ;
			}
			return 1 ;
		}
	}
	return 0 ;
}

// Get a frame from live stream
// return:
//		-1 = error
//		frame type
int net_live_getframe( SOCKET so, char ** framedata, int * framesize )
{
	struct Answer_type ans ;
	if( net_wait(so, RECV_TIMEOUT)==1 ) {
		if( net_recv( so, &ans, sizeof(ans))) {
			if( ans.anscode == ANSSTREAMDATA ) {
				if( ans.anssize<2000000 && ans.anssize>0 ) {
					* framedata = new char [ans.anssize] ;
					if( net_recv(so, *framedata, ans.anssize) ) {
						* framesize=ans.anssize ;
						return ans.data ;
					}
					delete *framedata ;
				}
			}
		}
	}
	return -1 ;
}

// int get DVR share root directory and password,
// parameter:
//		root: minimum size MAX_PATH bytes
int net_getshareinfo(SOCKET so, char * root )
{
	struct Req_type req ;
	struct Answer_type ans ;
	int res=0 ;
	char serveripname[128] ;		// DVR server ip name
	char password[128] ;
	struct sockad sad ;

	net_clear(so);		// clear received buffer

	memset( &sad, 0, sizeof(sad));
	sad.len = sizeof(sad)-sizeof(sad.len) ;

	// get DVR server ip name and share root
	getpeername(so, &sad.addr, &sad.len);
	net_sockname(serveripname, &sad);
	sprintf(root, "\\\\%s\\share$", serveripname );

	// get DVR server share password
	strcpy(password, "dvruser");	// default password
	req.reqcode = REQSHAREPASSWD ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSHAREPASSWD && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize>128 )  {
					ans.anssize=128 ;
				}
				strncpy( password, pbuf, 128 );
				res = 1 ;
			}
			delete pbuf ;
		}
	}


	USE_INFO_2 useinfo ;
	DWORD error ;
	wchar_t wpasswd[128] ;
	wchar_t wremote[MAX_PATH] ;
	memset( &useinfo, 0, sizeof(useinfo));

	MultiByteToWideChar(CP_ACP, 0, root, (int)strlen(root)+1, wremote, MAX_PATH);
	MultiByteToWideChar(CP_ACP, 0, password, (int)strlen(password)+1, wpasswd, 128 );

	useinfo.ui2_remote = (LMSTR)wremote ;
	useinfo.ui2_username = (LMSTR)(L"dvruser");
	useinfo.ui2_password = wpasswd;
	error=0;
	NetUseAdd(NULL, 2, (LPBYTE)&useinfo, &error);

	return res ;
}

// open a play back stream
//   return stream handle
//          -1 for error ;
int net_stream_open(SOCKET so, int channel)
{
	int res = -1 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSTREAMOPEN ;
	req.data = channel ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSTREAMOPEN ) {
			res=ans.data ;
		}
	}
	return res ;
}

// close playback stream
// return 1: success
//        0: error
int net_stream_close(SOCKET so, int handle)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSTREAMCLOSE ;
	req.data = handle ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res=1 ;
		}
	}
	return res ;
}

// playback stream seek
// return : 
//		  0: no encrypted stream
//		 -1: error 
//		other: RC4 key hash value
int net_stream_seek(SOCKET so, int handle, struct dvrtime * pseekto )
{
	int res = -1 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSTREAMSEEK ;
	req.data = handle ;
	req.reqsize = sizeof(*pseekto) ;
	net_send( so, &req, sizeof(req) );
	net_send( so, pseekto, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			return (int)ans.data ;
		}
	}
	return res ;
}

// get playback stream data
// return 1: success
//        0: no data
int net_stream_getdata(SOCKET so, int handle, char ** framedata, int * framesize, int * frametype )
{
	struct Req_type req ;
	struct Answer_type ans ;
	struct hd_frame * pframe ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSTREAMGETDATA ;
	req.data = handle ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSTREAMDATA && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize) ) {
				*framedata = pbuf ;
				*framesize = ans.anssize ;
				pframe = (struct hd_frame *) pbuf;
				* frametype = FRAME_TYPE_UNKNOWN ;
				if( pframe->flag==1 ) {
					if( pframe->type == 3 ) {	// key frame
						* frametype =  FRAME_TYPE_KEYVIDEO ;
					}
					else if( pframe->type == 1 ) {
						* frametype = FRAME_TYPE_AUDIO ;
					}
					else if( pframe->type == 4 ) {
						* frametype = FRAME_TYPE_VIDEO ;
					}
				}
				return 1 ;
			}
			delete pbuf ;
		}
	}
	return 0 ;
}

// get stream time
// return 1: success
// retur  0: failed
int net_stream_gettime(SOCKET so, int handle, struct dvrtime * t)
{
	struct dvrtime * pdtime;
	struct Req_type req ;
	struct Answer_type ans ;
	int res=0 ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSTREAMTIME ;
	req.data = handle ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSTREAMTIME && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				pdtime = (struct dvrtime *) pbuf ;
				t->year = pdtime->year ;
				t->month = pdtime->month ;
				t->day = pdtime->day ;
				t->hour = pdtime->hour ;
				t->min = pdtime->min ;
				t->second = pdtime->second ;
				t->millisecond=pdtime->millisecond ;
				t->tz = 0;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

// get stream day data availability information in a month
// return
//         bit mask of month info
//         0 for no data in this month
DWORD net_stream_getmonthinfo(SOCKET so, int handle, struct dvrtime * month)
{
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSTREAMMONTHINFO ;
	req.data = handle ;
	req.reqsize = sizeof(* month) ;
	net_send( so, &req, sizeof(req) );
	net_send( so, month, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSTREAMMONTHINFO ) {
			return (DWORD) ans.data ;
		}
	}
	return 0 ;
}

// get data availability information of a day
// return 
//        sizeof tinfo item received ( *tinfo will be allocated )
int net_stream_getdayinfo( SOCKET so, int handle, struct dvrtime * day, struct cliptimeinfo ** tinfo )
{
	struct Req_type req ;
	struct Answer_type ans ;
	int res=0 ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSTREAMDAYINFO ;
	req.data = handle ;
	req.reqsize = sizeof(* day) ;
	net_send( so, &req, sizeof(req) );
	net_send( so, day, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSTREAMDAYINFO && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			res = ans.anssize / sizeof( struct cliptimeinfo ) ;
			* tinfo = new struct cliptimeinfo [res+1] ;
			if( net_recv(so, *tinfo, ans.anssize ) ) {
				return res ;
			}
			delete [] *tinfo ;
			*tinfo=NULL ;
			res = 0;
		}
	}
	return res;
}

// get locked file availability information of a day
// return 
//        sizeof tinfo item received
int net_stream_getlockfileinfo(SOCKET so, int handle, struct dvrtime * day, struct cliptimeinfo ** tinfo )
{
	struct Req_type req ;
	struct Answer_type ans ;
	int res=0 ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQLOCKINFO ;
	req.data = handle ;
	req.reqsize = sizeof(* day) ;
	net_send( so, &req, sizeof(req) );
	net_send( so, day, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSTREAMDAYINFO && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			res = ans.anssize / sizeof( struct cliptimeinfo ) ;
			* tinfo = new struct cliptimeinfo [res+1] ;
			if( net_recv(so, *tinfo, ans.anssize ) ) {
				return res ;
			}
			delete [] *tinfo ;
			*tinfo = NULL ;
			res = 0 ;
		}
	}
	return res;
}


// get DVR system time
// return 
//        1 success, DVR time in *t
//        0 failed
int net_getdvrtime(SOCKET so, struct dvrtime * t )
{
	struct Req_type req ;
	struct Answer_type ans ;
	int res=0 ;
	SYSTEMTIME * psystm ;
	struct dvrtime * pdvrt ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQ2GETLOCALTIME ;
	req.data = 0 ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANS2TIME && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				pdvrt = (struct dvrtime *)pbuf ;
				t->year=pdvrt->year ;
				t->month = pdvrt->month ;
				t->day = pdvrt->day ;
				t->hour = pdvrt->hour ;
				t->min = pdvrt->min ;
				t->second = pdvrt->second ;
				t->millisecond = pdvrt->millisecond;
				res=1 ;
			}
			delete pbuf ;
		}
	}
	if( res ) {
		return res ;
	}

	// Try Use REQGETLOCALTIME
	req.reqcode = REQGETLOCALTIME ;
	req.data = 0 ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETTIME && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				psystm = (SYSTEMTIME *)pbuf ;
				t->year=psystm->wYear ;
				t->month = psystm->wMonth ;
				t->day = psystm->wDay ;
				t->hour = psystm->wHour ;
				t->min = psystm->wMinute ;
				t->second = psystm->wSecond ;
				t->millisecond = psystm->wMilliseconds ;
				res=1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

// get DVR system setup
int net_GetSystemSetup(SOCKET so, void * buf, int bufsize)
{
	int res = 0 ;

	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETSYSTEMSETUP ;
	req.data = 0 ;
	req.reqsize = 0;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSSYSTEMSETUP && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize>bufsize ) {
					ans.anssize = bufsize ;
				}
				memcpy( buf, pbuf, ans.anssize ) ;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

int net_SetSystemSetup(SOCKET so, void * buf, int bufsize)
{
	int res=0;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSETSYSTEMSETUP ;
	req.data = 0 ;
	req.reqsize = bufsize;
	net_send( so, &req, sizeof(req) );
	net_send( so, buf, bufsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

int net_GetChannelSetup(SOCKET so, void * buf, int bufsize, int channel)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;
	req.data = channel;
	req.reqcode = REQGETCHANNELSETUP ;
	req.reqsize = 0 ;

	net_clear(so);		// clear received buffer

	net_send(so, &req, sizeof(req));
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSCHANNELSETUP && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize>bufsize ) {
					ans.anssize = bufsize ;
				}
				memcpy( buf, pbuf, ans.anssize ) ;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

int net_SetChannelSetup(SOCKET so, void * buf, int bufsize, int channel)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.data=channel;
	req.reqcode = REQSETCHANNELSETUP ;
	req.reqsize = bufsize ;
	net_send( so, &req, sizeof(req) );
	net_send( so, buf, bufsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

int net_DVRGetChannelState(SOCKET so, void * buf, int bufsize )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETCHANNELSTATE ;
	req.data = 16 ;
	req.reqsize = 0 ;
	net_send(so, &req, sizeof(req));
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETCHANNELSTATE && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize>bufsize ) {
					ans.anssize = bufsize ;
				}
				memcpy( buf, pbuf, ans.anssize ) ;
				res = ans.anssize ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

int net_DVRGetTimezone( SOCKET so, TIME_ZONE_INFORMATION * tzi )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETTIMEZONE ;
	req.data = 0 ;
	req.reqsize = 0 ;

	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSTIMEZONEINFO && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize>sizeof(*tzi) ) {
					ans.anssize = sizeof(*tzi) ;
				}
				memcpy( tzi, pbuf, ans.anssize ) ;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

int net_DVRSetTimezone( SOCKET so, TIME_ZONE_INFORMATION * tzi )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSETTIMEZONE ;
	req.data = 0 ;
	req.reqsize = sizeof( TIME_ZONE_INFORMATION ) ;

	net_send( so, &req, sizeof(req) );
	net_send( so, tzi, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

int net_DVRGetLocalTime(SOCKET so, SYSTEMTIME * st )
{
	int res ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETLOCALTIME ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETTIME && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize>sizeof(*st) ) {
					ans.anssize = sizeof(*st) ;
				}
				memcpy( st, pbuf, ans.anssize ) ;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

int net_DVRSetLocalTime(SOCKET so, SYSTEMTIME * st )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSETLOCALTIME ;
	req.data = 0 ;
	req.reqsize = sizeof( SYSTEMTIME ) ;
	net_send( so, &req, sizeof(req) );
	net_send( so, st, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

int net_DVRGetSystemTime(SOCKET so, SYSTEMTIME * st )
{
	int res ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETSYSTEMTIME ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETTIME && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize>sizeof(*st) ) {
					ans.anssize = sizeof(*st) ;
				}
				memcpy( st, pbuf, ans.anssize ) ;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

int net_DVRSetSystemTime(SOCKET so, SYSTEMTIME * st )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSETSYSTEMTIME ;
	req.data = 0 ;
	req.reqsize = sizeof( SYSTEMTIME ) ;
	net_send( so, &req, sizeof(req) );
	net_send( so, st, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

// Get time zone info
// return buffer size returned, return 0 for error
int net_DVR2ZoneInfo( SOCKET so, char ** zoneinfo)
{
	int res =0;
	struct Req_type req ;
	struct Answer_type ans ;

	req.reqcode = REQ2GETZONEINFO ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANS2ZONEINFO && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				*zoneinfo = pbuf ;
				res = ans.anssize ;
			}
			else {
				delete pbuf ;
			}
		}
	}
	return res ;
}

// Set DVR time zone
//   timezone: timezone name to set
// return 1:success, 0:fail
int net_DVR2SetTimeZone(SOCKET so, char * timezone)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQ2SETTIMEZONE ;
	req.data = 0 ;
	req.reqsize = (int)strlen( timezone )+1 ;
	net_send( so, &req, sizeof(req) );
	net_send( so, timezone, req.reqsize );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

// Get DVR time zone
// return buffer size returned, return 0 for error
//    timezone: DVR timezone name 
int net_DVR2GetTimeZone( SOCKET so, char ** timezone )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQ2GETTIMEZONE ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANS2TIMEZONE && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				*timezone = pbuf ;
				res = ans.anssize ;
			}
			else {
				delete pbuf ;
			}
		}
	}
	return res ;
}

// get dvr (version2) time
//    st: return dvr time
//    system: 0-get localtime, 1-get system time
// return: 0: fail, 1: success
int net_DVR2GetTime(SOCKET so, SYSTEMTIME * st, int system)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;
	struct dvrtime * pdvrt ;

	net_clear(so);		// clear received buffer

	if( system )
		req.reqcode = REQ2GETSYSTEMTIME ;
	else 
		req.reqcode = REQ2GETLOCALTIME ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANS2TIME && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				pdvrt = (struct dvrtime *)pbuf ;
				st->wYear = pdvrt->year ;
				st->wMonth = pdvrt->month ;
				st->wDay = pdvrt->day ;
				st->wDayOfWeek=0;
				st->wHour = pdvrt->hour ;
				st->wMinute = pdvrt->min ;
				st->wSecond = pdvrt->second ;
				st->wMilliseconds = pdvrt->millisecond ;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

// set dvr (version2) time
//    st: return dvr time
//    system: 0-set localtime, 1-set system time
// return: 0: fail, 1: success
int net_DVR2SetTime(SOCKET so, SYSTEMTIME * st, int system)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;
	struct dvrtime dvrt ;

	net_clear(so);		// clear received buffer

	if( system )
		req.reqcode = REQ2SETSYSTEMTIME ;
	else 
		req.reqcode = REQ2SETLOCALTIME ;
	req.data = 0 ;
	req.reqsize = sizeof( dvrt ) ;
	net_send(so, &req, sizeof(req));
	dvrt.year=st->wYear;
	dvrt.month=st->wMonth;
	dvrt.day=st->wDay;
	dvrt.hour=st->wHour;
	dvrt.min=st->wMinute;
	dvrt.second=st->wSecond;
	dvrt.millisecond=0 ;
	net_send(so, &dvrt, sizeof(dvrt));
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

// get dvr timezone information entry
// return: 0: fail, 1: success
int net_GetDVRTZIEntry(SOCKET so, void * buf, int bufsize )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETTZIENTRY ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSTZENTRY && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize > bufsize ) {
					ans.anssize = bufsize ;
				}
				memcpy( buf, pbuf, ans.anssize );
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

// Kill DVR application, if Reboot != 0 also reset DVR
int net_DVRKill(SOCKET so, int Reboot )
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQKILL ;
	if( Reboot ) {
		req.data = -1 ;
	}
	else {
		req.data = 0 ;
	}
	req.reqsize = 0 ;
	net_send(so, &req, sizeof(req));
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ) {
			res = 1 ;
		}
	}
	return res ;
}

int net_GetSetupPage(SOCKET so, char * pagebuf, int bufsize)
{
	int res = 0 ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQ2GETSETUPPAGE ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANS2SETUPPAGE && ans.anssize<MAX_ANSSIZE && ans.anssize>0 ) {
			char * pbuf = new char [ans.anssize] ;
			if( net_recv(so, pbuf, ans.anssize ) ) {
				if( ans.anssize > bufsize ) {
					ans.anssize = bufsize ;
				}
				memcpy( pagebuf, pbuf, ans.anssize );
				pagebuf[bufsize-1]=0 ;
				res = 1 ;
			}
			delete pbuf ;
		}
	}
	return res ;
}

// check TVS key data. Return 1: success
int net_CheckKey(SOCKET so, char * keydata)
{
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	struct key_data *kd = (struct key_data *)keydata ;
	req.reqcode = REQCHECKKEY ;
	req.data = 0 ;
	req.reqsize = 128+kd->size ;
	net_send( so, &req, sizeof(req) );
	net_send( so, keydata, req.reqsize );

	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ){
			return 1 ;
		}
	}
	return 0 ;
}


/*

// get key (TVS/PWII) access log data. Return 1: success, return 0: failed
int net_GetKeyLog(SOCKET so, char ** keylog, int * datalen)
{
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETTVSLOG ;
	req.data = 0 ;
	req.reqsize = 0 ;
	net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSGETTVSLOG && ans.anssize>0 && ans.anssize<1000000 ) {
			*keylog = (char *)malloc( ans.anssize );
			if( *keylog ) {
				net_recv( so, *keylog, ans.anssize );
				* datalen = ans.anssize ;
				return 1 ;
			}
		}
	}
	return 0 ;
}
*/

// Send keypad info to PWII/PW3
int net_sendpwkeypad(SOCKET so, int keycode, int press )
{
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQ2KEYPAD ;
	req.reqsize = 0 ;
	req.data = (keycode&0xff) ;
    if( press ) {
        req.data|=0x100 ;
    }
    net_send( so, &req, sizeof(req) );
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ){
			return 1 ;
		}
	}
	return 0 ;
}


// Send dvr data
int net_senddvrdata( SOCKET so, int protocol, void * sendbuf, int sendsize)
{
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQSENDDATA ;
	req.reqsize = sendsize ;
	req.data = protocol ;
    net_send( so, &req, sizeof(req) );
    if( sendsize>0 ) {
        net_send( so, sendbuf, sendsize );
    }
	if( net_recv( so, &ans, sizeof(ans))) {
		if( ans.anscode == ANSOK ){
			return 1 ;
		}
	}
	return 0 ;
}

// Receiving dvr data
int net_recvdvrdata( SOCKET so, int protocol,  void ** precvbuf, int * precvsize)
{
    int res = 0 ;
    void * rbuf ;
	struct Req_type req ;
	struct Answer_type ans ;

	net_clear(so);		// clear received buffer

	req.reqcode = REQGETDATA ;
	req.reqsize = 0 ;
	req.data = protocol ;
    net_send( so, &req, sizeof(req) );
    if( net_recv( so, &ans, sizeof(ans))) {
        if( ans.anssize>0 ) {
            rbuf = malloc( ans.anssize );
            int rsize = net_recv(so, rbuf, ans.anssize );
            if( ans.anscode == ANSGETDATA && rsize>0 ) {
                if( precvbuf ) {
                    *precvbuf = rbuf ;
                    rbuf = NULL ;           // caller should call freedvrdata() later to free this buffer
                }
                if( precvsize ) {
                    *precvsize=rsize ;
                }
                res = rsize ;
            }
            if( rbuf ) {
                free(rbuf);
            }
        }
	}
	return res ;
}

