#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <stdlib.h>
#define CONST MAX_PATH = 200
using namespace std;
char dir[MAX_PATH];
char** signatures = NULL;
int* signszs = NULL;
int offsets[] = { 0x400, 0x400 };
int nsigns = 2;

bool Check(char* name_file, char* signature, int signsz, int offset)
{
    FILE* fp;
    fopen_s(&fp, name_file, "rb");
    if (fp == NULL)
    {
        cout << "\nCan't open file\n";
        return NULL;
    }

    fseek(fp, offset, SEEK_SET);
    char* data_file = (char*)malloc(signsz * sizeof(char));
    fread_s(data_file, signsz, sizeof(char), signsz, fp);
    fclose(fp);
    for (int i = 0; i < signsz; i++)
    {
        if (data_file[i] != signature[i])
        {
            return false;
        }
    }
    
    return true;
}

int fix1(char* exename)
{
    FILE* fp;
    fopen_s(&fp, exename, "rb");
    if (fp == NULL)
    {
        printf("Can't open file %s\n", exename);
        return 1;
    }
    long pos;
    unsigned char length;
    fseek(fp, 0x8e1, SEEK_SET);
    fread(&pos, sizeof(long), sizeof(char), fp);
    printf("text offset: %x\n", pos);
    fseek(fp, 0x8ed, SEEK_SET);
    fread(&length, sizeof(char), sizeof(char), fp);
    printf("text length: %x\n", length);
    fseek(fp, pos, SEEK_SET);
    char* textbuffer = (char*)malloc((length) * sizeof(char));
    fread_s(textbuffer, length, sizeof(char), length, fp);
    fclose(fp);

    char name_split[MAX_PATH];
    _splitpath_s(exename, NULL, 0, NULL, 0, name_split, MAX_PATH, NULL, 0);

    char txtname[MAX_PATH];
    sprintf_s(txtname, MAX_PATH, "%s%s.txt", dir, name_split);

    fp = NULL;
    fopen_s(&fp, txtname, "wb");
    if (fp == NULL)
    {
        printf("Can't create file %s\n", txtname);
        return 1;
    }
    fwrite(textbuffer, sizeof(char), length, fp);
    fclose(fp);

    free(textbuffer);
    remove(exename);
    return 0;
}

int fix2(char* exename)
{
    char name_split[MAX_PATH];
    _splitpath_s(exename, NULL, 0, NULL, 0, name_split, MAX_PATH, NULL, 0);

    char txtname[MAX_PATH];
    sprintf_s(txtname, MAX_PATH, "C:\\Lab3\\Temp\\%s.txt", name_split);

    char fixtxt[MAX_PATH];
    sprintf_s(fixtxt, MAX_PATH, "%s%s.txt", dir, name_split);

    if (!CopyFileA(txtname, fixtxt, FALSE))
    {
        std::cout << "Fail to copy: " << txtname << std::endl;
        return 1;
    }

    remove(exename);
    return 0;
}

int ListDirectoryContents()
{
    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;
    char mask[MAX_PATH];
    strcpy(mask, dir);
    strcat(mask, "*.exe");
    printf("mask %s\n", mask);

    if ((hFind = FindFirstFileA(mask, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    do
    {
        char filename[MAX_PATH];
        strcpy_s(filename, MAX_PATH, dir);
        strcat_s(filename, MAX_PATH, fdFile.cFileName);
        
        printf("Checking %s for signature 1\n", filename);
        if (Check(filename, signatures[0], signszs[0], offsets[0]))
        {
            if (!fix1(filename))
            {
                printf("Successfully fixed %s\n", filename);
                continue;
            }
        }

        printf("Checking %s for signature 2\n", filename);
        if (Check(filename, signatures[1], signszs[1], offsets[1]))
        {
            if (!fix2(filename))
            {
                printf("Successfully fixed %s\n", filename);
            }
        }

    } while (FindNextFileA(hFind, &fdFile));

    FindClose(hFind);
    return 0;
}

int GetSignature(char name[], char** signature)
{
    FILE* fp;
    fopen_s(&fp, name, "rb");
    if (fp == NULL)
    {
        cout << "\nCan't open file\n";
        return 0;
    }
    fseek(fp, 0L, SEEK_END);
    long txtsz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    *signature = (char*)malloc(txtsz * sizeof(char));
    fread_s(*signature, txtsz, sizeof(char), txtsz, fp);
    fclose(fp);
    return txtsz;
}

int main()
{
    char exename[MAX_PATH];
    GetModuleFileNameA(NULL, exename, MAX_PATH);

    char name_split[MAX_PATH];
    char dir_split[MAX_PATH];
    char drive_split[10];
    _splitpath_s(exename, drive_split, 10, dir_split, MAX_PATH, name_split, MAX_PATH, NULL, 0);

    sprintf_s(dir, MAX_PATH, "%s%s", drive_split, dir_split);
    printf("Directory: %s\n", dir);

    char sig[MAX_PATH];
    strcpy(sig, dir);
    strcat(sig, "signature");

    char sig2[MAX_PATH];
    strcpy(sig2, dir);
    strcat(sig2, "signature2");

    signatures = (char**)malloc(nsigns * sizeof(char*));
    signszs = (int*)malloc(nsigns * sizeof(int));
    signszs[0] = GetSignature(sig, &signatures[0]);
    signszs[1] = GetSignature(sig2, &signatures[1]);
    printf("Signature read: %s\n", sig);
    printf("Signature read: %s\n", sig2);
    ListDirectoryContents();
    system("pause");
    return 0;
}