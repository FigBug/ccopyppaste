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

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <stdio.h>

int _tmain(int argc, _TCHAR* argv[])
{
   BOOL bToScreen = (argc == 2 && !strcmp(argv[1], "-v")) ? TRUE : FALSE;

   OpenClipboard(NULL);

   HANDLE hClipData;

   BOOL bSuccess   = FALSE;
   BOOL bOperation = DROPEFFECT_COPY;

   // Get The Drop Effect
   hClipData = GetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT));
   if (hClipData)
   {
      DWORD* pDropEffect = (DWORD*)GlobalLock(hClipData);
      if (*pDropEffect == DROPEFFECT_MOVE)
         bOperation = DROPEFFECT_MOVE;

      GlobalUnlock(hClipData);
   }

   // HDROP
   hClipData = GetClipboardData(CF_HDROP);
   if (hClipData && !bSuccess)
   {
      DROPFILES* pDropFiles = (DROPFILES*)GlobalLock(hClipData);
      int cnt = DragQueryFile((HDROP)hClipData, -1, NULL, 0);
      for (int i = 0; i < cnt; i++)
      {
         char szFilename[MAX_PATH];
         DragQueryFile((HDROP)hClipData, i, szFilename, sizeof(szFilename));
         if (bToScreen)
         {
            puts(szFilename);
         }
         else
         {
            char szFrom[MAX_PATH + 1] = {0};
            char szTo[MAX_PATH + 1]   = {0};

            strcpy(szFrom, szFilename);
            GetCurrentDirectory(sizeof(szTo), szTo);
            
            SHFILEOPSTRUCT shfo;
            ZeroMemory(&shfo, sizeof(shfo));
            shfo.hwnd   = NULL;
            shfo.wFunc  = (bOperation == DROPEFFECT_COPY) ? FO_COPY : FO_MOVE;
            shfo.pFrom  = szFrom;
            shfo.pTo    = szTo;
            shfo.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;

            SHFileOperation(&shfo);

            EmptyClipboard();
         }
      }
      bSuccess = TRUE;
   }

   // Text  
   hClipData = GetClipboardData(CF_TEXT);
   if (hClipData && !bSuccess)
   {
      char* pText = (char*)GlobalLock(hClipData);
      puts(pText);
      GlobalUnlock(hClipData);
      bSuccess = TRUE;
   }

   CloseClipboard();

	return 0;
}

