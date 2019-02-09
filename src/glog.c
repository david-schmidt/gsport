/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2019 by GSport contributors
 Copyright (C) 2016 - 2018  Dagen Brock
 
 Based on the KEGS emulator written by and Copyright (C) 2003 Kent Dickey

 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2 of the License, or (at your 
 option) any later version.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 for more details.

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "glog.h"

int glog(const char *s) {
  time_t timer;
  char buffer[26];
  struct tm* tm_info;

  time(&timer);
  tm_info = localtime(&timer);

  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  printf("%s - %s\n", buffer, s);

  return 0;
}


int glogf(const char *fmt, ...) {

  time_t timer;
  char buffer[26];
  struct tm* tm_info;

  time(&timer);
  tm_info = localtime(&timer);

  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

  printf("%s - ", buffer);

  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  fputc('\n', stdout);
  return 0;
}
