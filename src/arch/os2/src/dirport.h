/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef VICE_DIRPORT_H
#define VICE_DIRPORT_H

#define INCL_DOS
#include <os2.h>
#include <malloc.h>
#include <sys/stat.h>

#ifndef _A_VOLID
#define _A_VOLID 0
#endif

#ifndef _A_NORMAL
#define _A_NORMAL FILE_NORMAL
#endif

#ifndef _A_RDONLY
#define _A_RDONLY FILE_READONLY
#endif

#ifndef _A_HIDDEN
#define _A_HIDDEN FILE_HIDDEN
#endif

#ifndef _A_SYSTEM
#define _A_SYSTEM FILE_SYSTEM
#endif

#ifndef _A_SUBDIR
#define _A_SUBDIR FILE_DIRECTORY
#endif

#ifndef _A_ARCH
#define _A_ARCH FILE_ARCHIVED
#endif

#define _A_ANY FILE_NORMAL | FILE_READONLY  | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY | FILE_ARCHIVED

#ifndef EPERM
#define EPERM EDOM // Operation not permitted = Domain Error
#endif

#define dirent _FILEFINDBUF3
#define d_name achName     /* For struct dirent portability    */
#define d_size cbFile

#define mkdir(name, mode) mkdir(name)

#ifndef WATCOM_COMPILE
#define S_ISDIR(mode) ((mode) & S_IFDIR)

typedef struct _DIR {
    struct dirent buffer;
    HDIR handle;
    APIRET ulrc;
} DIR;

extern DIR *opendir(char *path);
extern struct dirent *readdir(DIR *dirp);
extern int closedir(DIR *dirp);
#endif

#endif /* DIRPORT__H */
