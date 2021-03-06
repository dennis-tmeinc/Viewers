
#ifndef ___CLASS_STRING___
#define ___CLASS_STRING___

#include <windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#pragma warning (disable : 4996 )

class string {
protected:
	char * pstr;
	WCHAR * pwcs;

	// memory allocations
	static void * _alloc(int size) {
		return malloc(size);
	}

	static void * _realloc(void * p, int size) {
		return realloc(p, size);
	}

	static void _free(void *p) {
		free(p);
	}

	static int _msize(void *p) {
		return (int)::_msize(p);
	}

	char * allocstr(int size) {
		return (pstr = (char *)_realloc(pstr, size * sizeof(char)));
	}

	WCHAR * allocwcs(int size) {
		return (pwcs = (WCHAR *)_realloc(pwcs, size * sizeof(WCHAR)));
	}

	void freestr()
	{
		if (pstr) {
			_free(pstr);
			pstr = NULL;
		}
	}

	void freewcs()
	{
		if (pwcs) {
			_free(pwcs);
			pwcs = NULL;
		}
	}

	void set(const char * str) {
		freewcs();
		if (pstr != str) {
			if (str == NULL || *str == 0) {
				freestr();
			}
			else {
				char * nstr = (char *)_alloc(lstrlenA(str) + 2);
				lstrcpyA(nstr, str);
				freestr();
				pstr = nstr;
			}
		}
	}

	void set(const WCHAR * wcs) {
		freestr();
		if (pwcs != wcs) {
			if (wcs == NULL || *wcs == 0 ) {
				freewcs();
			}
			else {
				WCHAR * nwcs = (WCHAR *)_alloc((lstrlenW(wcs)+2)*sizeof(WCHAR));
				lstrcpyW(nwcs, wcs);
				freewcs();
				pwcs = nwcs;
			}
		}
	}

public:
	string() :                        // default constructor
		pstr(NULL),
		pwcs(NULL)
	{
	}

	string(const char * str) :
		pstr(NULL),
		pwcs(NULL)
	{
		set(str);
	}

	string(const WCHAR * wcs) :
		pstr(NULL),
		pwcs(NULL)
	{
		set(wcs);
	}

	string(string & s) :
		pstr(NULL),
		pwcs(NULL)
	{
		*this = s;
	}

	~string() {
		clean();
	}

	void clean() {
		freestr();
		freewcs();
	}

	string & operator = (const char * str) {
		set(str);
		return *this;
	}

	string & operator = (const WCHAR * str) {
		set(str);
		return *this;
	}

	string & operator = (string & s) {
		if (s.pstr) {
			set(s.pstr);
		}
		else if (s.pwcs) {
			set(s.pwcs);
		}
		else {
			clean();
		}
		return *this;
	}

	int isempty() {
		if (pstr) return (*pstr == 0);
		if (pwcs) return (*pwcs == 0);
		return 1;
	}

	operator char * () {
		if (pstr == NULL) {
			if (pwcs) {
				int sizestr = WideCharToMultiByte(CP_UTF8, 0, pwcs, -1, NULL, 0, NULL, NULL);
				allocstr(sizestr+1);
				WideCharToMultiByte(CP_UTF8, 0, pwcs, -1, pstr, sizestr, NULL, NULL);
				freewcs();
			}
			else {
				*allocstr(4) = 0;
			}
		}
		return pstr;
	}

	operator WCHAR * () {
		if (pwcs == NULL) {
			if (pstr) {
				int sizewcs = MultiByteToWideChar(CP_UTF8, 0, pstr, -1, NULL, 0);
				allocwcs(sizewcs+1);
				MultiByteToWideChar(CP_UTF8, 0, pstr, -1, pwcs, sizewcs);
				freestr();
			}
			else {
				*allocwcs(4) = 0;
			}
		}
		return pwcs;
	}

	static int compare(const char * s1, const char *s2)
	{
		return lstrcmpA(s1, s2);
	}
	
	static int compare(const WCHAR * s1, const WCHAR *s2)
	{
		return lstrcmpW(s1, s2);
	}

	int compare(const char * str) {
		return compare(str, *this);
	}

	int compare(const WCHAR * wcs) {
		return compare(wcs, *this);
	}

	int compare(string & s) {
		if (s.pwcs) {
			return compare(s.pwcs);
		}
		else if (s.pstr) {
			return compare(s.pstr);
		}
		else {
			return !isempty();
		}
	}

	int operator > (const char * str) {
		return compare(str) > 0;
	}

	int operator > (const WCHAR * wcs) {
		return compare(wcs) > 0;
	}

	int operator > (string & str) {
		return compare(str) > 0;
	}

	int operator < (const char * str) {
		return compare(str) < 0;
	}

	int operator < (const WCHAR * wcs) {
		return compare(wcs) < 0;
	}

	int operator < (string & str) {
		return compare(str) < 0;
	}

	int operator == (const char * str) {
		return compare(str) == 0;
	}

	int operator == (const WCHAR * wcs) {
		return compare(wcs) == 0;
	}

	int operator == (string & str) {
		return compare(str) == 0;
	}

	// increase string buffer size 
	char * strsize(int newsize) {
		if( _msize((char *)*this) <= newsize)
			allocstr(newsize + 2);
		return pstr;
	}

	// increase wide string buffer size 
	WCHAR * wcssize(int newsize) {
		if (_msize((WCHAR *)*this)/(int)sizeof(WCHAR) <= newsize)
			allocwcs(newsize + 2);
		return pwcs;
	}

	inline TCHAR * tcssize(int size) {
#ifdef  UNICODE
		// unicode
		return wcssize(size);
#else
		return strsize(size);
#endif
	}

	string & operator += (const char * str) {
		if (str != NULL) {
			lstrcatA(strsize(lstrlenA(*this) + lstrlenA(str) + 2), str);
		}
		return (*this);
	}

	string & operator += (const WCHAR * wcs) {
		if (wcs != NULL) {
			lstrcatW(wcssize(lstrlenW(*this) + lstrlenW(wcs) + 2), wcs);
		}
		return (*this);
	}

	string & operator += ( string & s ) {
		if (s.pwcs) {
			(*this) += s.pwcs;
		}
		else if (s.pstr) {
			(*this) += s.pstr;
		}
		return (*this);
	}

	string & trimtail() {
		char * t = (char *)(*this);
		int l = lstrlenA(t);

		// trim tail
		while (l > 0) {
			if (t[l - 1] > ' ' || t[l - 1] < 0) {
				break;
			}
			l--;
		}
		if (t[l])
			t[l] = 0;
		return (*this);
	}

	string & trimhead() {
		char * t = (char *)(*this);
		// trim head
		while (*t <= ' ' && *t > 0) {
			t++;
		}

		if (t != pstr)
			set(t);
		return (*this);
	}

	string & trim() {
		trimtail();
		return trimhead();
	}

	string & printf(const char * format, ...) {
		va_list va;
		va_start(va, format);
		vsnprintf(strsize(2048), 2040, format, va);
		va_end(va);
		return *this;
	}

};

// temperary formated string
#define TMPSTR(...)		string().printf(__VA_ARGS__) 

#endif  // ___CLASS_STRING___
