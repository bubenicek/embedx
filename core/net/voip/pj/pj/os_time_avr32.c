/* $Id: os_time_unix.c 2394 2008-12-23 17:27:53Z bennylp $ */
/*
 * Copyright (C) 2008-2009 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <pj/os.h>
#include <pj/errno.h>
#include <pj/string.h>
#include <pj/compat/time.h>

#if defined(PJ_HAS_UNISTD_H) && PJ_HAS_UNISTD_H!=0
#    include <unistd.h>
#endif

#include <errno.h>
#include <sys/time.h>
#include <clock.h>
#include <debug.h>

///////////////////////////////////////////////////////////////////////////////

PJ_DEF(pj_status_t) pj_gettimeofday(pj_time_val *p_tv)
{
    p_tv->sec = clock_time() / 1000;
    p_tv->msec = 0;
    return PJ_SUCCESS;
}

