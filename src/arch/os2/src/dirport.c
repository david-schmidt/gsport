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
 
#include <stdio.h>
#include <string.h>
#include "dirport.h"

DIR *opendir (char *path)
{
    APIRET ulrc;
    ULONG cnt = 1;
    DIR *dir;
    char *name;

    if (!(dir = (DIR*)calloc(1, sizeof(DIR)))) {
        return NULL;
    }
    if (name = (char*)calloc(1, strlen(path) + 3)) {
        strcat(strcpy(name, path),path[strlen(path) - 1] == '\\' ? "*" : "\\*");
        dir->handle = HDIR_CREATE;
        ulrc = DosFindFirst(name, &(dir->handle), _A_ANY, &(dir->buffer), sizeof(struct _FILEFINDBUF3), &cnt, FIL_STANDARD);
        lib_free(name);
    }
    if (!name || ulrc) {
        DosFindClose (dir->handle);
        lib_free(dir);
        dir = NULL;
    }
    return dir;
}

struct dirent *readdir (DIR *dir)
{
    ULONG cnt = 1;

    return DosFindNext (dir->handle, &(dir->buffer), sizeof(struct _FILEFINDBUF), &cnt) ? NULL : &(dir->buffer);
}

int closedir (DIR *dir)
{
    APIRET ulrc = DosFindClose (dir->handle);

    lib_free(dir);
    return (int)ulrc;
}
