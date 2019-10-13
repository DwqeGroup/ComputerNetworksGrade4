#include "h.h"
//Base64转明文
char table[] =
{
	'A' , 'B' , 'C' , 'D' , 'E' , 'F' , 'G',
	'H' , 'I' , 'J' , 'K' , 'L' , 'M' , 'N',
	'O' , 'P' , 'Q' , 'R' , 'S' , 'T' , 'U',
	'V' , 'W' , 'X' , 'Y' , 'Z' , 'a' , 'b',
	'c' , 'd' , 'e' , 'f' , 'g' , 'h' , 'i',
	'j' , 'k' , 'l' , 'm' , 'n' , 'o' , 'p',
	'q' , 'r' , 's' , 't' , 'u' , 'v' , 'w',
	'x' , 'y' , 'z' , '0' , '1' , '2' , '3',
	'4' , '5' , '6' , '7' , '8' , '9' , '+',
	'/' , '='
};
char* base64_decode(const char *src)
{
	int count = 0, len = 0;
	char *dst = NULL;

	int tmp = 0, buf = 0;
	int i = 0, j = 0, k = 0;
	char in[5] = { 0 };
	len = strlen(src);
	count = len / 4;

	dst = (char *)malloc(count * 3 + 1);
	memset(dst, 0, count * 3 + 1);

	for (j = 0; j < count; j++)
	{
		memset(in, 0, sizeof(in));
		strncpy_s(in, src + j * 4, 4);

		buf = 0;
		for (i = 0; i < 4; i++)
		{
			tmp = (long)in[i];
			if (tmp == '=')
			{
				tmp = 0;
			}
			else
			{
				for (k = 0; ; k++)
				{
					if (table[k] == tmp)
						break;
				}
				tmp = k;
			}
			tmp <<= (18 - i * 6);
			buf |= tmp;

		}

		for (i = 0; i < 3; i++)
		{
			tmp = buf >> (16 - i * 8);
			tmp &= 0xff;
			dst[j * 3 + i] = tmp;
		}
	}

	return dst;
}
char* Utf8ToAscii(const char* szU8) //以\0结尾
{
	//先转换为Unicode字符串
	//预转换，得到所需空间的大小;
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, 0, szU8, strlen(szU8), NULL, 0);
	//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	if (!wszString)
		return NULL;
	//转换
	::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen);
	//最后加上'\0'
	wszString[wcsLen] = '\0';
	//转换为Ascii字符串
	int asciilen = WideCharToMultiByte(CP_ACP, 0, wszString, -1, NULL, 0, '\0', NULL);
	char* ascii = new char[asciilen + 1];
	if (!ascii)
	{
		delete[]wszString;
		return NULL;
	}
	WideCharToMultiByte(CP_ACP, 0, wszString, -1, ascii, asciilen, '\0', NULL);
	ascii[asciilen] = '\0';
	delete[]wszString;
	return ascii;
}