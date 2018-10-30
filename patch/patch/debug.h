/**
  * libcrippy-1.0 - debug.h
  * Copyright (C) 2013 Crippy-Dev Team
  * Copyright (C) 2010-2013 Joshua Hill
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <syslog.h>

#ifdef _DEBUG
#define info(...) syslog(LOG_ERR, __VA_ARGS__)
#define error(...) syslog(LOG_ERR, __VA_ARGS__)
#define debug(...) syslog(LOG_ERR, __VA_ARGS__)
#else
#define info(...) syslog(LOG_ERR, __VA_ARGS__)
#define error(...) syslog(LOG_ERR, __VA_ARGS__)
#define debug(...)
#endif

void hexdump(unsigned char* buf, unsigned int len);

#endif /* DEBUG_H_ */
