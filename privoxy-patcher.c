// -----------------------------------------------------------
// original code in privoxy.exe:
//   8DB426 00000000     LEA ESI,DWORD PTR DS:[ESI]
//   897424 04           MOV DWORD PTR SS:[ESP+4],ESI
//   BE 01000000         MOV ESI,1
//   893C24              MOV DWORD PTR SS:[ESP],EDI
//   E8 8BF00000         CALL <JMP.&USER32.ShowWindow>
// -----------------------------------------------------------
// patch code setting nCmdShow to SW_HIDE:
//   33F6                XOR ESI,ESI       ; ESI = 0 (SW_HIDE)
//   EB 03               JMP SHORT privoxy.0042CE90
//   90                  NOP
//   90                  NOP
//   90                  NOP
//   897424 04           MOV DWORD PTR SS:[ESP+4],ESI
//   BE 01000000         MOV ESI,1
//   893C24              MOV DWORD PTR SS:[ESP],EDI
//   E8 8BF00000         CALL <JMP.&USER32.ShowWindow>   
// -----------------------------------------------------------

#include <stdio.h>
#include <windows.h>

const unsigned char* original   = "\x8d\xb4\x26\x00\x00\x00\x00\x89\x74\x24\x04\xbe\x01\x00\x00\x00\x89\x3c\x24";
const unsigned char* patch      = "\x33\xf6\xeb\x03\x90\x90\x90\x89\x74\x24\x04\xbe\x01\x00\x00\x00\x89\x3c\x24";
const unsigned int   patchlen   = 19;
const unsigned int   bufsz      = 0x80000;

int main(int argc, char** argv) {
    HANDLE hFile;                       // handle to privoxy.exe
    DWORD dwBytesRead = 0;              // size of privoxy.exe
    unsigned int patcher = 0;           // offset for patching
    unsigned int it_buf = 0;            // buf iterator
    unsigned char buf[0x80000] = {0};   // buffer for contents of privoxy.exe

    // -----------------------------------------------------------
    // check usage and bail if missing argument
    // -----------------------------------------------------------
    if (argc != 2) {
        fprintf(stderr, "[x] Usage: %s <privoxy.exe>\n", argv[0]);
        return 1;
    }

    // -----------------------------------------------------------
    // open privoxy binary for reading
    // -----------------------------------------------------------
    hFile = CreateFile(argv[1], (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[x] Unable to open file: %s\n", argv[1]);
        return 2;
    }

    // -----------------------------------------------------------
    // read privoxy binary into buf
    // -----------------------------------------------------------
    if(FALSE == ReadFile(hFile, buf, bufsz-1, &dwBytesRead, NULL)) {
        fprintf(stderr, "[x] Unable to read file: %s\n", argv[1]);
        CloseHandle(hFile);
        return 3;
    }

    // -----------------------------------------------------------
    // iterate across file contents
    // -----------------------------------------------------------
    for (it_buf = 0; it_buf < dwBytesRead; it_buf++) {

        // -----------------------------------------------------------
        // examine current byte vs. original target byte
        // continue while current bytes match original target bytes
        // -----------------------------------------------------------
        if (*(unsigned char*)(buf + it_buf) == *(unsigned char*)original) {
            for (patcher = 0; patcher < patchlen; patcher++) {
                if (*(unsigned char*)(buf + it_buf + patcher) != *(unsigned char*)(original + patcher)) {
                    break;
                }
            }
            
            // -----------------------------------------------------------
            // found the original bytes, apply the patch
            // -----------------------------------------------------------
            if (patcher == patchlen) {
                printf("[+] Found patch target at 0x%08x\n", it_buf);
                for (patcher = 0; patcher < patchlen; patcher++) {
                    *(unsigned char*)(buf + it_buf + patcher) = *(unsigned char*)(patch + patcher);
                }
                printf("[+] Patched in memory, writing to file... ");
                break;
            }
        }
    }

    // -----------------------------------------------------------
    // write the patched buffer back to the file
    // -----------------------------------------------------------
    if (patcher == patchlen) {
        if (0 != SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
            fprintf(stderr, "\n[x] Could not reset file pointer for patch.\n");
            return 4;
        }

        if (FALSE == WriteFile(hFile, buf, dwBytesRead, &dwBytesRead, NULL)) {
            fprintf(stderr, "\n[x] Could not write patch to file.\n");
            CloseHandle(hFile);
            return 5;
        }

        printf("complete!\n");    
    }

    // -----------------------------------------------------------
    // original bytes to patch were not found
    // -----------------------------------------------------------
    else {
        fprintf(stderr, "[x] Target bytes to patch could not be found.\n");
        return 6;
    }

    CloseHandle(hFile);
    return 0;
}