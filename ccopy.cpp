/*
Copyright (c) 2005-2012: Roland Rabien
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FileGlobBase.h"
#include "FileGlobList.h"
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>

using namespace std;
#include <vector>

void Glob(const char* pszText, vector<char*>& files)
{
   FileGlobList glob;
   glob.MatchPattern(pszText);

   for (FileGlobList::Iterator it = glob.begin(); it != glob.end(); it++)
	{
		const char* str = (*it).c_str();

      char* pszFilePart = NULL;
      char szFullName[MAX_PATH + 1];
      GetFullPathName(str, sizeof(szFullName), szFullName, &pszFilePart);

      if (GetFileAttributes(szFullName) != INVALID_FILE_ATTRIBUTES)
         files.push_back(_strdup(szFullName));
	}   
}

int _tmain(int argc, _TCHAR* argv[])
{
   vector<char*> text;
   vector<char*> files;

   if (argc == 1)
   {
      char szBuffer[1024];
      while (fgets(szBuffer, sizeof(szBuffer), stdin))
      {
         // copy the text
         text.push_back(_strdup(szBuffer));

         char* c;
         while (c = strchr(szBuffer, '\n')) *c = 0;
         // copy the filenames
         Glob(szBuffer, files);
      }
   }
   else
   {
      // Get all the file names
      FileGlobList glob;
      for (int i = 1; i < argc; i++)
         Glob(argv[i], files);
   }

   // Copy it to the clipboard
   OpenClipboard(NULL);
   EmptyClipboard();

   // if the is stdin text, copy it to the clipboard
   if (text.size())
   {
      // Copy the text
      int nTextSize = 0;
      for (int i = 0; i < int(text.size()); i++)
         nTextSize += int(strlen(text[i]));
      nTextSize++;

      HGLOBAL hGlobText = GlobalAlloc(GHND, nTextSize);
      char* pszText = (char*)GlobalLock(hGlobText);

      char* p = pszText;
      for (int i = 0; i < int(text.size()); i++)
         p += sprintf(p, "%s", text[i]);

      GlobalUnlock(hGlobText);

      SetClipboardData(CF_TEXT, hGlobText);
   }
   else
   {
      // Copy the text
      int nTextSize = 0;
      for (int i = 0; i < int(files.size()); i++)
         nTextSize += int(strlen(files[i]) + 2);
      nTextSize--;

      HGLOBAL hGlobText = GlobalAlloc(GHND, nTextSize);
      char* pszText = (char*)GlobalLock(hGlobText);

      char* p = pszText;
      for (int i = 0; i < int(files.size() - 1); i++)
         p += sprintf(p, "%s\r\n", files[i]);
      sprintf(p, "%s", files[files.size() - 1]);

      GlobalUnlock(hGlobText);

      SetClipboardData(CF_TEXT, hGlobText);
   }

   if (files.size() > 0)
   {
      // Copy files to the clipboard
      int nDropSize = sizeof(DROPFILES);
      for (int i = 0; i < int(files.size()); i++)
         nDropSize += int(strlen(files[i]) + 1);
      nDropSize++;

      HGLOBAL hGlobDrop = GlobalAlloc(GHND, nDropSize);
      char* pDrop = (char*)GlobalLock(hGlobDrop);

      DROPFILES* pDropFiles = (DROPFILES*)pDrop;
      ZeroMemory(pDropFiles, sizeof(DROPFILES));
      pDropFiles->pFiles = sizeof(DROPFILES);
      pDropFiles->pt.x   = 0;
      pDropFiles->pt.y   = 0;
      pDropFiles->fNC    = FALSE;
      pDropFiles->fWide  = FALSE;

      pDrop += sizeof(DROPFILES);
      for (int i = 0; i < int(files.size()); i++)
      {
         strcpy(pDrop, files[i]);
         pDrop += strlen(pDrop) + 1;
      }
      *pDrop = '\0';

      GlobalUnlock(hGlobDrop);

      SetClipboardData(CF_HDROP, hGlobDrop);

      // Prefered drop effect
      HGLOBAL hGlobDropEffect = GlobalAlloc(GHND, sizeof(DWORD));
      
      DWORD* pDropEffect = (DWORD*)GlobalLock(hGlobDropEffect);
#ifdef CUT
      *pDropEffect = DROPEFFECT_MOVE;
#else
      *pDropEffect = DROPEFFECT_COPY;
#endif

      GlobalUnlock(hGlobDropEffect);
      
      SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), hGlobDropEffect);
   }

   // Clpse the clipboard
   CloseClipboard();

	return 0;
}

