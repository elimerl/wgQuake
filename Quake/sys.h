/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2010-2014 QuakeSpasm developers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _QUAKE_SYS_H
#define _QUAKE_SYS_H

// sys.h -- non-portable functions

void Sys_Init(void);

// alternate user data dir when playing the 2021 re-release
// (to avoid writing in Nightdive Studios/Quake)
qboolean Sys_GetAltUserPrefDir(qboolean remastered, char *path,
                               size_t pathsize);

// shows path in file browser
qboolean Sys_Explore(const char *path);

//
// file IO
//

typedef int64_t qfileofs_t;

// returns the file size or -1 if file is not present.
// the file should be in BINARY mode for stupid OSs that care
qfileofs_t Sys_FileOpenRead(const char *path, int *hndl);

// Returns a file handle
int Sys_FileOpenWrite(const char *path);

void Sys_FileClose(int handle);
void Sys_FileSeek(int handle, int position);
int Sys_FileRead(int handle, void *dest, int count);
int Sys_FileWrite(int handle, const void *data, int count);
qboolean Sys_FileExists(const char *path);
qboolean Sys_GetFileTime(const char *path, time_t *out);
void Sys_mkdir(const char *path);
FILE *Sys_fopen(const char *path, const char *mode);
int Sys_fseek(FILE *file, qfileofs_t ofs, int origin);
qfileofs_t Sys_ftell(FILE *file);
int Sys_remove(const char *path);
int Sys_rename(const char *oldname, const char *newname);

typedef enum {
  FA_DIRECTORY = 1 << 0,
} fileattribs_t;

typedef struct findfile_s {
  fileattribs_t attribs;
  char name[MAX_OSPATH];
} findfile_t;

findfile_t *Sys_FindFirst(const char *dir, const char *ext);
findfile_t *Sys_FindNext(findfile_t *find);

// Only needs to be called manually when breaking out of the loop,
// otherwise the last Sys_FindNext will also close the handle
void Sys_FindClose(findfile_t *find);

int Sys_FileType(const char *path);
/* returns an FS entity type, i.e. FS_ENT_FILE or FS_ENT_DIRECTORY.
 * returns FS_ENT_NONE (0) if no such file or directory is present. */

qboolean Sys_IsDebuggerPresent(void);

void *Sys_LoadLibrary(const char *path);
void *Sys_GetLibraryFunction(void *lib, const char *func);
void Sys_CloseLibrary(void *lib);

//
// system IO
//
FUNC_NORETURN void Sys_Quit(void);
FUNC_NORETURN void Sys_ReportError(const char *error, ...) FUNC_PRINTF(1, 2);

#define Sys_Error(...)                                                         \
  do {                                                                         \
    if (Sys_IsDebuggerPresent())                                               \
      SDL_TriggerBreakpoint();                                                 \
    Sys_ReportError(__VA_ARGS__);                                              \
  } while (0)

// an error will cause the entire program to exit
#ifdef __WATCOMC__
#pragma aux Sys_Error aborts;
#pragma aux Sys_Quit aborts;
#endif

void Sys_Printf(const char *fmt, ...) FUNC_PRINTF(1, 2);
// send text to the console

double Sys_DoubleTime(void);

const char *Sys_ConsoleInput(void);

void Sys_Sleep(unsigned long msecs);
// yield for about 'msecs' milliseconds.

void Sys_SendKeyEvents(void);
// Perform Key_Event () callbacks until the input que is empty

void Sys_ActivateKeyFilter(qboolean active);

static inline qboolean Sys_IsPathSep(char c) {
#ifdef _WIN32
  return c == '/' || c == '\\';
#else
  return c == '/';
#endif
}

#endif /* _QUAKE_SYS_H */
