#include "exstring.h"

inline void _tcsaddquotes(TCHAR *str)
{
	_tcsremquotes(str);
	// Shift on one symbol
	TCHAR *pos;
	for (pos = str + _tcslen(str) + 1; pos > str; pos--) {
		*pos= *(pos - 1);
	}
	*str = '\"';
	_tcscat(str, "\"");
}

inline void _straddquotes(char *str)
{
	_strremquotes(str);
	// Shift on one symbol
	char *pos;
	for (pos = str + strlen(str) + 1; pos > str; pos--) {
		*pos= *(pos - 1);
	}
	*str = '\"';
	strcat(str, "\"");
}

inline void _tcsremquotes(TCHAR *str)
{
	// Find begin of word
	TCHAR *posStr, *posWord;
	for (posStr = str; *posStr == ' '; posStr++) {}

	// Remove quotes
	for (posWord = str; *posStr != '\0'; posStr++) {
		if (*posStr != '\"') {
			*posWord = *posStr;
			posWord++;
		}
	}
	*(posWord) = '\0';
}

inline void _strremquotes(char *str)
{
	// Find begin of word
	char *posStr, *posWord;
	for (posStr = str; *posStr == ' '; posStr++) {}

	// Remove quotes
	for (posWord = str; *posStr != '\0'; posStr++) {
		if (*posStr != '\"') {
			*posWord = *posStr;
			posWord++;
		}
	}
	*(posWord) = '\0';
}

inline void _tcsextrword(TCHAR *str, TCHAR *extrWord)
{
	*extrWord = '\0';
	// Find begin of word
	TCHAR *posStr, *posWord;
	for (posStr = str; *posStr == ' '; posStr++) {}

	// Copy word with quotes
	bool inquote = false;
	for (posWord = extrWord; (*posStr != ' ' || inquote) && *posStr != '\0'; posStr++, posWord++) {
		if (*posStr == '\"') {
			inquote = !inquote;
		}
		*posWord = *posStr;
	}
	*posWord = '\0';
}

inline void _strextrword(char *str, char *extrWord)
{
	*extrWord = '\0';
	// Find begin of word
	char *posStr, *posWord;
	for (posStr = str; *posStr == ' '; posStr++) {}

	// Copy word with quotes
	bool inquote = false;
	for (posWord = extrWord; (*posStr != ' ' || inquote) && *posStr != '\0'; posStr++, posWord++) {
		if (*posStr == '\"') {
			inquote = !inquote;
		}
		*posWord = *posStr;
	}
	*posWord = '\0';
}

inline TCHAR *_tcsdelword(TCHAR *str, TCHAR *delsubstrword) {

	TCHAR *posBegin, *posEnd;

	// Extract one word or words in quotes from delsubstr
	TCHAR *findWord, *delWord;
	delWord = new TCHAR[_tcslen(delsubstrword) + 1];
	_tcsextrword(delsubstrword, delWord);

	// Find begin of word
	if ((findWord = posBegin = _tcsstr(str, delWord)) == NULL) {
		delete[] delWord;
		return NULL;
	}
	delete[] delWord;

	// Find end of word
	bool inquote = false;
	for (posEnd = posBegin; (*posEnd != ' ' || inquote) && *posEnd != '\0'; posEnd++) {
		if (*posEnd == '\"') inquote = !inquote;
	}

	// Find next word 
	for (; *posEnd == ' '; posEnd++) {}

	// Move characters
	for (; *posEnd != '\0'; posBegin++, posEnd++){
		*posBegin = *posEnd;
	}
	// Terminate string
	*posBegin = '\0';

	return findWord;
}

inline char *_strdelword(char *str, char *delsubstrword) {

	char *posBegin, *posEnd;

	// Extract one word or words in quotes from delsubstr
	char *findWord, *delWord;
	delWord = new char[strlen(delsubstrword) + 1];
	_strextrword(delsubstrword, delWord);

	// Find begin of word
	if ((findWord = posBegin = strstr(str, delWord)) == NULL) {
		delete[] delWord;
		return NULL;
	}
	delete[] delWord;

	// Find end of word
	bool inquote = false;
	for (posEnd = posBegin; (*posEnd != ' ' || inquote) && *posEnd != '\0'; posEnd++) {
		if (*posEnd == '\"') inquote = !inquote;
	}

	// Find next word 
	for (; *posEnd == ' '; posEnd++) {}

	// Move characters
	for (; *posEnd != '\0'; posBegin++, posEnd++){
		*posBegin = *posEnd;
	}
	// Terminate string
	*posBegin = '\0';

	return findWord;
}
