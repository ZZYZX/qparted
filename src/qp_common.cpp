/*
    qtparted - a frontend to libparted for manipulating disk partitions
    Copyright (C) 2002-2003 Vanni Brutto

    Vanni Brutto <zanac (-at-) libero dot it>

    Copyright (C) 2005-2010 Bernhard Rosenkraenzer <bero@arklinux.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <sys/param.h>  // MAXPATHLEN
#include <sys/stat.h>   // S_ISLNK
#include <unistd.h>     // readlink
#include <dirent.h>
#include <stdio.h>

#include "qp_common.h"

bool flagDevfsEnabled;

QP_ListExternalTools *lstExternalTools = new QP_ListExternalTools();

/*---this function test if the kernel support devfs.
 *   the code was bring from partimage software made by Fran?ois Dupoux---*/
bool isDevfsEnabled() {
	flagDevfsEnabled=!access("/dev/.devfsd", F_OK);
	return flagDevfsEnabled;
}
