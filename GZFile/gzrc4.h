#if !defined (UCHAR)
#define UCHAR unsigned char
#endif

#if !defined (PSZ)
#define PSZ char*
#endif


void RC4InitSBox (PSZ pszKey);

PSZ RC4CryptStream (PSZ pszOut, PSZ pszIn, int iSrcLen);
