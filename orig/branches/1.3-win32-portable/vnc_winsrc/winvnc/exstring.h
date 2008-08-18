#ifndef _EXSTRING_H_
#define _EXSTRING_H_

#include <string.h>
#include <tchar.h>

extern inline void _tcsaddquotes(TCHAR *str);
extern inline void _tcsremquotes(TCHAR *str);
extern inline int _tcsextrword(TCHAR *str, TCHAR *extrWord);
extern inline TCHAR *_tcsdelword(TCHAR *str, TCHAR *delsubstrword);

extern inline void _straddquotes(char *str);
extern inline void _strremquotes(char *str);
extern inline int _strextrword(char *str, char *extrWord);
extern inline char *_strdelword(char *str, char *delsub_strword);
extern char *_strCharToHexStr(const char *mem, const int len);
extern char *_strHexStrToChar(const char *hexStr, int *len);
extern void _strXORString(char *str, const int lenStr, const char *key, const int lenKey);

#endif // _EXSTRING_H_