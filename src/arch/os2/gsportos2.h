/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 by GSport contributors
 
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

/*
 * Function prototypes
 */
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
VOID AbortHello(HWND hwndFrame, HWND hwndClient);

#define MSGBOXID    1001

#define ID_WINDOW   256

#define ID_OPTIONS  257
#define ID_OPTION1  258
#define ID_OPTION2  259
#define ID_OPTION3  260

#define ID_EXITPROG 261

#define IDS_HELLO   262
#define IDS_1       263
#define IDS_2       264
#define IDS_3       265