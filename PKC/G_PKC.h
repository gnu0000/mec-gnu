 //
//
// CPKC.h
//	Public Key Cryptography class	1.0
//

#if !defined (CPKC_INCLUDED)
#define CPKC_INCLUDED
#pragma once

#include "PGPAll.h"

//	typedef for callback function while a key is being generated
//
typedef int (*EVENTFN) (int iType, int iVal);

// flag values used by AddEntropy()
//
#define PGP_EN_MOUSE	   				0x01
#define PGP_EN_SYSTEM					0x02

// option values used by ImportKey()
//
#define PGPIMPORT_FORMAT_COMPATIBLE	0
#define PGPIMPORT_FORMAT_PEM			1
#define PGPIMPORT_FORMAT_PKCS7		2
#define PGPIMPORT_FORMAT_PKCS8		3
#define PGPIMPORT_FORMAT_PKCS12     4


// default filenames
//
#define FILE_DEFAULT_PUBRING "pubring.pkr"
#define FILE_DEFAULT_SECRING "secring.skr"


// possible error values in addition to standard PGP error values
//
#define PKCERR_NO_KEYRING				   1
#define PKCERR_NO_FILE					   2
#define PKCERR_NO_ID						   3
#define PKCERR_NO_PWD					   4
#define PKCERR_NO_BUFFER				   5
#define PKCERR_NO_KEY_MATCHES			   6
#define PKCERR_NO_DIR					   7
#define PKCERR_BAD_PASSWORD            8
#define PKCERR_BLOCK_IS_SIGNATURE      9
#define PKCERR_BLOCK_IS_NOT_SIGNATURE  10
#define PKCERR_DID_NOT_DECRYPT         11
#define PKCERR_MAX_ERROR				   PKCERR_DID_NOT_DECRYPT


//
//	CPKC
//	Class to support Key Management, Encrypting/Decrypting and Signing
//
class CPKC
	{
public:
   // state information gathered by Decode() callback handler
   // these need to be public cause they are used by callbacks
   BOOL  m_bGotDecryptionEvent;
   BOOL  m_bGotSignatureEvent ;
   BOOL  m_bGotPassphraseEvent;
   char  m_szSignKeyID [256]  ;


	// create / init
	CPKC ();
	CPKC (PSZ pszPubRingFile, PSZ pszSecRingFile, BOOL bUserLocalDir = TRUE, BOOL bReadOnly = FALSE);
	~CPKC ();

	// Error Handling
	virtual int IsError (PSZ* ppszErr = NULL);
	virtual PSZ GetError (int* piError = NULL);

	// local keyring open/close
	int  OpenKeyRing (PSZ pszPubRingFile, PSZ pszSecRingFile, BOOL bUserLocalDir = TRUE, BOOL bReadOnly = FALSE);
	int  CloseKeyRing ();

	// Encrypting
	int  ClearEncryptKeys ();
	int  AddEncryptKeyFromKeyRing (PSZ pszID);
	int  AddEncryptKeyFromBuffer (PSZ pszPubKeyBuf);
	int  EncryptFile (PSZ pszDestFile, PSZ pszSrcFile, BOOL bAsciiArmor = TRUE);

	// Decrypting (assumes decrypt key in keyring)
	int  DecryptFile (PSZ pszDestFile, PSZ pszSrcFile, PSZ pszPWD);

	// Signing (assumes signer key in keyring)
	int  SignFile (PSZ pszDestFile, PSZ pszSrcFile, PSZ pszSignID, PSZ pszSignPWD, BOOL bSignInClear = FALSE);

   // Extract Signature
   int  ExtractSignature (PSZ pszDestFile, PSZ pszSrcFile, PSZ pszSignKeyID);

	//	Key Management
	int  CreateKey (PSZ pszID, PSZ pszPWD, EVENTFN pfnEvent = NULL);
	int  DeleteKey (PSZ pszID);
	int  ExportKey (PSZ pszKeyID, PSZ pszKeyFile, BOOL bInclPrivate = FALSE, PSZ pszExportMsg = NULL);
	int  ImportKey (PSZ pszKeyFile, int iFormatFlag = PGPIMPORT_FORMAT_COMPATIBLE, PSZ pszPWD = NULL, PSZ pszImportedKeyIDName = NULL); // flag: PGPIMPORT_FORMAT_*
   int  ImportKey (BYTE* pbKeyBuffer, int iKeyBufferLen, int iFormatFlag, PSZ pszPWD);  // flag: PGPIMPORT_FORMAT_*

	// Entropy (needed for creating keys and adding attributes)
	int  GetEntropyAvailable ();					  // How much entropy do we have
	int  GetEntropyNeeded ();						  // How much entropy do we need to make a key?
	int  AddEntropy (int iEntropyTypeFlag = 0); // Add some entropy (PGP_EN_MOUSE | PGP_EN_SYSTEM)
   int  GuaranteeMinimumEntropy ();            // Ensure we have enough to do an entropy consuming operation

	// Key utility functions
	int  KeyExists (PSZ pszID, BOOL* pbExists); // this function is case sensitive, so BIDDER will not match bidder
	int  GetKeyMatchCount (PSZ pszIDMatchSpec, int* piMatchCount, BOOL bExact = FALSE);
	int  GetKeyMatchList (PSZ pszIDMatchSpec, CStringArray* pstrMatchList);
	int  GetKeyFingerprint (PSZ pszID, PSZ pszFingerprint);
	int  GetKeySetIDList (PGPKeySetRef hKeySet, CStringArray* pstrIDList);
   int  TestKeyFromBuffer (PSZ pszPubKeyBuf, CStringArray* pstrIDList);
   int  GetKeyNotation (PSZ pszID, PSZ pszNotation);
   int  SetKeyNotation (PSZ pszID, PSZ pszPWD, PSZ pszNotation);
   int  KeyInfo (PSZ pszID, BOOL bExact);


protected:
	int  				m_iError			;
	PGPContextRef  m_hContext		;
	PGPKeyDBRef    m_hKeyDB       ;
	PGPKeyDBRef    m_hEncryptKeyDB;
	static char    m_szErrorBuffer [256];
	BOOL				m_bSeedFileInit;

   int  Init ();
	int  FormatFingerprint (PSZ pszFingerprint, PGPByte* pbFingerprint, PGPSize iLength);
	int  InitSeedFile ();
	PGPOptionList* InputFormatOption (int iFormatFlag);
	PGPError GetMatchingKeys (PSZ pszID, PGPKeySetRef* phKeySet, BOOL bExact = FALSE);
   PGPError GetMatchingKey (PSZ pszID, PGPKeyDBObjRef* phKey, BOOL bExact = FALSE);
	int  SetSpecialFilePath (PSZ pszDestPath, PSZ pszSrcFile, BOOL bUserLocalDir = TRUE);

   void BoolProperty (int iIndent, PGPKeyDBObjRef hKey, PGPKeyDBObjProperty iProperty, PSZ pszStr);
   void NumProperty (int iIndent, PGPKeyDBObjRef hKey, PGPKeyDBObjProperty iProperty, PSZ pszStr);
   int  _keyInfo (int iIndent, PGPKeyDBObjRef hKey, BOOL bPrimary);
   int  _subkeyInfo (int iIndent, PGPKeyDBObjRef hKey);


	virtual int  SetError (int iErr);
	virtual PSZ  GetErrorString ();
	};


#endif // !defined (CPKC_INCLUDED)


