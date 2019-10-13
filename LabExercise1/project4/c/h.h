#ifndef OKL_____________L
#define OKL_____________L
#pragma once
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <shlwapi.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma warning(disable:4996)
//Base64¼ÓÃÜ½âÃÜ
char* base64_decode(const char *src);
#else 
#endif
//UTF8×ªASCIIÂë
char* Utf8ToAscii(const char* szU8);