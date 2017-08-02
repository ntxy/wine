/*
 * NLS Related Functions
 *
 * Copyright 2016
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"

#include <ctype.h>
#include <stdarg.h>

#define WINE_NO_INLINE_STRING
#include "windef.h"
#include "winbase.h"
#include "winnls.h"
#include "wine/debug.h"
#include "wine/unicode.h"
#include "wine/exception.h"

WINE_DEFAULT_DEBUG_CHANNEL(nls);


static WCHAR *strdupW(const WCHAR* str)
{
    WCHAR *ret;
    DWORD size = (strlenW(str) + 1) * sizeof(WCHAR);

    ret = HeapAlloc(GetProcessHeap(), 0, size);
    if(ret)
        memcpy(ret, str, size);
    return ret;
}


static WCHAR* _wcsrev( WCHAR* str )
{
  WCHAR* ret = str;
  WCHAR* end = str + strlenW(str) - 1;
  while (end > str)
  {
    WCHAR t = *end;
    *end--  = *str;
    *str++  = t;
  }
  return ret;
}


int WINAPI FindNLSStringEx(
                           LPCWSTR          lpLocaleName,
                           DWORD            dwFindNLSStringFlags,
                           LPCWSTR          lpStringSource,
                           int              cchSource,
                           LPCWSTR          lpStringValue,
                           int              cchValue,
                           LPINT            pcchFound,
                           LPNLSVERSIONINFO lpVersionInformation,
                           LPVOID           lpReserved,
                           LPARAM           sortHandle
                           ) {

  WCHAR* source;
  WCHAR* value;
  LPCWSTR result;

  int ret = -1;


  SetLastError(ERROR_SUCCESS);


  source = strdupW(lpStringSource);
  if (!source) {
    return -1;
  }

  value = strdupW(lpStringValue);
  if (!value) {
    HeapFree(GetProcessHeap(), 0, source);
    return -1;
  }


  if (cchSource == -1) {
    cchSource = strlenW(lpStringSource);
  }

  if (cchValue == -1) {
    cchValue = strlenW(lpStringValue);
  }


  if (dwFindNLSStringFlags & LINGUISTIC_IGNORECASE ||
      dwFindNLSStringFlags & NORM_IGNORECASE) {
    strlwrW(source);
    strlwrW(value);
  }


  if (dwFindNLSStringFlags & FIND_FROMEND ||
      dwFindNLSStringFlags & FIND_ENDSWITH) {
    _wcsrev(source);
    _wcsrev(value);
  }


  result = strstrW(source, value);


  if (dwFindNLSStringFlags & FIND_STARTSWITH ||
      dwFindNLSStringFlags & FIND_ENDSWITH) {
    if (result == source) {
      ret = 0;
    } else {
      ret = -1;
    }
  } else if (dwFindNLSStringFlags & FIND_FROMSTART) {
    if (result) {
      ret = result - source;
    } else {
      ret = -1;
    }
  } else if (dwFindNLSStringFlags & FIND_FROMEND) {
    if (result) {
      printf("%i %i\n", result - source, cchSource);
      ret = cchSource - 1 - (result - source);
    } else {
      ret = -1;
    }
  } else {
    SetLastError(ERROR_INVALID_PARAMETER);
    ret = -1;
  }


  WINE_TRACE("flags = %i, haystack = %s, needle = %s -> ret = %i\n",
             dwFindNLSStringFlags,
             debugstr_w(lpStringSource),
             debugstr_w(lpStringValue),
             ret);


  HeapFree(GetProcessHeap(), 0, source);
  HeapFree(GetProcessHeap(), 0, value);


  return ret;
}
