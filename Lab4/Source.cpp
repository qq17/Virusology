#include <iostream>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <stdlib.h>
#include <string>
#define CONST MAX_PATH = 200
#define _CRT_SECURE_NO_WARNINGS
using namespace std;
char dir[MAX_PATH];

// Проверка сигнатура находится ли в рассмотрелном файле
bool Check(char* name_file, char* signature)
{
    FILE* fp;
    fopen_s(&fp, name_file, "rb");
    if (fp == NULL)
    {
        cout << "\nCan't open file";
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    long txtsz = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    char* data_file = (char*)malloc(txtsz * sizeof(char));
    fread_s(data_file, txtsz, sizeof(char), txtsz, fp);
    fclose(fp);
    string data = data_file, sig = signature;
    return data.find(sig);
}
int fix(char* exename)
    {   
    //Прочитать файл exe
    FILE* fp;
    fopen_s(&fp, exename, "rb");
    if (fp == NULL)
    {
        printf("Can't open file %s\n", exename);
        return 1;
    }
    long pos, length;
    int res;
    //Найти начальный адресс текста exe файла
    fseek(fp, 0x8e1, SEEK_SET);
    fread(&pos, sizeof(long), sizeof(char), fp);
    // Найти длина текста, чуть я изменил здесть прочитал 4 байта
    fseek(fp, 0x8ed, SEEK_SET);
    fread(&length, sizeof(long), sizeof(char), fp);
    // Прочитай текста exe файла
    fseek(fp, pos, SEEK_SET);
    int i = 0;
    char* textfile = (char*)malloc((length) * sizeof(char));
    while (i < length) {
        int p;
        fread(&p, 1, 1, fp);
        textfile[i] = (char)p;
        i++;
    }

    fclose(fp);
    int ret = remove(exename);
    
    //Создай тектовый файл
    char name_split[MAX_PATH];
    char dir_split[MAX_PATH];
    char drive_split[10];
    char txtname[MAX_PATH];
    // Создай новый пусть тектового файла
    _splitpath_s(exename, drive_split, 10, dir_split, MAX_PATH, name_split, MAX_PATH, NULL, 0);

    sprintf_s(dir, MAX_PATH, "%s%s", drive_split, dir_split);
    sprintf_s(txtname, MAX_PATH, "%s%s.txt", dir, name_split);
    // Запись текст в файл
    fp = NULL;
    fopen_s(&fp, txtname, "w");
    if (fp == NULL)
    {
        printf("\nCan't create text file %s", txtname);
        return 1;
    }
    for (int i = 0; i < length; i++) {
        fputc(textfile[i], fp);
    }
    fclose(fp);
    free(textfile);
    return 0;
}

int ListDirectoryContents(char * signature)
{
    WIN32_FIND_DATAW fdFile;
    HANDLE hFind = NULL;
    //Создать путь директории
    wchar_t mask[MAX_PATH];
    char mask_[MAX_PATH];
    char pathfile[MAX_PATH];
    strcpy(mask_, dir);
    strcat(mask_, "/*.exe");
    mbstowcs(mask, mask_, MAX_PATH);
    
    if ((hFind = FindFirstFileW(mask, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    do
    {
        // Создать путь файла
        char filename[MAX_PATH];
        wcstombs(filename, fdFile.cFileName, MAX_PATH);
        strcpy_s(pathfile,MAX_PATH, dir);
        strcat_s(pathfile, "/");
        strcat_s(pathfile,MAX_PATH, filename);
        //Проверка
        if (Check(pathfile, signature))
        {
            if (!fix(pathfile)) {
                printf("Suceess fixed %s", pathfile);
            }
        }
       
       
    } while (FindNextFileW(hFind, &fdFile));

    FindClose(hFind);

    return 0;
}


//Прочитай сигнатуру
char* GetSignature(char name[])
{
    FILE* fp;
    fopen_s(&fp, name, "rb");
    if (fp == NULL)
    {
        cout << "\nCan't open file";
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    long txtsz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* signature = (char*)malloc(txtsz * sizeof(char));
    fread_s(signature, txtsz, sizeof(char), txtsz, fp);
    fclose(fp);
    return signature;
}

int main() {
    char sig[MAX_PATH];
    strcpy(sig, "signature");
    char* signature = GetSignature(sig);
    // Плучение путь данной папки
    DWORD res;
    res = GetCurrentDirectoryA(MAX_PATH, dir);
    ListDirectoryContents(signature);
    return 0;
}