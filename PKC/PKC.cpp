//
//
// CPKC.cpp
//
//
#include "..\stdafx.h"
#include "shlobj.h"
#include "pgpall.h"
#include "G_PKC.h"


// These defines are needed because the static version of the PGP
// libraries are broken.
//
extern "C"
   {
   unsigned __int32 IsdGetCapability   (unsigned __int32 in, unsigned __int32 *out) { return -1; }
   unsigned __int32 IsdGetRandomNumber (unsigned __int32 *out)                      { return -1; }
   };


#define PGP_LOCAL_DATA_DIR   "PGPData"
#define PGP_ENTROPY_FILE     "randpool.rnd"

#define KEYSIZE 1024

#define RETURN_IF_ERROR(iError) {int i=(iError); if(i) return SetError (i);}
#define RETURNV_IF_ERROR(iError) {int i=(iError); if(i) {SetError (i); return;}}
#define RETURN_IF_ASSERTERROR(bTest,iError) {BOOL b=(bTest); if (!b) return SetError(iError);}

// Additional Error Strings
//
static char *ppszCPKC_ERRORS[] = 
   {"No Error",                   // (string order must match define values in CPKC.h)
    "Could notIfind keyring",     // PKCERR_NO_KEYRING      
    "Filename not specified",     // PKCERR_NO_FILE         
    "Key ID not specified",       // PKCERR_NO_ID            
    "Password not specified",     // PKCERR_NO_PWD         
    "Buffer not specified",       // PKCERR_NO_BUFFER      
    "No key matches",             // PKCERR_NO_KEY_MATCHES
    "No user Directory",          // PKCERR_NO_DIR   
    "Bad Password",               // PKCERR_BAD_PASSWORD          
    "Block is a signature",       // PKCERR_BLOCK_IS_SIGNATURE    
    "Block is not a signature",   // PKCERR_BLOCK_IS_NOT_SIGNATURE
    "Data did not decrypt",       // PKCERR_DID_NOT_DECRYPT       
    NULL};

char CPKC::m_szErrorBuffer [256];

//////////////////////////////////////////////////////////////////////////////

// class constructor
//
CPKC::CPKC ()
   {
   Init ();
   }


// class constructor
// also opens the keyring files
//
CPKC::CPKC (PSZ pszPubDBFile, PSZ pszSecDBFile, BOOL bUserLocalDir, BOOL bReadOnly)
   {
   if (!Init ())
      OpenKeyRing (pszPubDBFile, pszSecDBFile, bUserLocalDir, bReadOnly);
   }


int CPKC::Init ()
   {
   m_iError        = 0;
   m_hContext      = kInvalidPGPContextRef;
   m_hKeyDB        = kInvalidPGPKeyDBRef;
   m_hEncryptKeyDB = kInvalidPGPKeyDBRef;
   m_bSeedFileInit = FALSE;

   RETURN_IF_ERROR (PGPsdkInit (kPGPFlags_ForceLocalExecution));
   RETURN_IF_ERROR (PGPNewContext (kPGPsdkAPIVersion, &m_hContext));

   // PGPSetRandSeedFileLoc
   return 0;
   }


// class destructor
//
CPKC::~CPKC ()
   {
   ClearEncryptKeys ();
   CloseKeyRing ();

   if (PGPContextRefIsValid (m_hContext))
      PGPFreeContext (m_hContext);

   PGPsdkCleanup();
   }


//////////////////////////////////////////////////////////////////////////////

// error value/string access 
//
int CPKC::IsError (PSZ* ppszErr)
   {
   if (ppszErr)
      *ppszErr = GetErrorString ();
   return m_iError;
   }


// error value/string access 
//
PSZ CPKC::GetError (int* piError)
   {
   if (piError)
      *piError = m_iError;
   return GetErrorString ();
   }


// internal get error access
//
PSZ CPKC::GetErrorString ()
   {
   if (m_iError < 0) // standard PGP Error
      {
      PGPGetErrorString(m_iError, sizeof(CPKC::m_szErrorBuffer), CPKC::m_szErrorBuffer);
      return CPKC::m_szErrorBuffer;
      }
   if (m_iError <= PKCERR_MAX_ERROR) // Additional CPKC Errors
      return ppszCPKC_ERRORS[m_iError];

   return "Unknown Error";
   }


// internal set error status
//
int CPKC::SetError (int iErr)
   {
   return (m_iError = iErr);
   }

//////////////////////////////////////////////////////////////////////////////


// opens local keyring files and sets key database handle
//
int CPKC::OpenKeyRing (PSZ pszPubDBFile, PSZ pszSecDBFile, BOOL bUserLocalDir, BOOL bReadOnly)
   {
   PGPFileSpecRef hPubFile = kInvalidPGPFileSpecRef;
   PGPFileSpecRef hSecFile = kInvalidPGPFileSpecRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszPubDBFile && *pszPubDBFile && pszSecDBFile && *pszSecDBFile, PKCERR_NO_FILE);

   // make sure we don't have a keyDB opened already
   if (m_hKeyDB != kInvalidPGPKeyDBRef)
      RETURN_IF_ERROR (PGPFreeKeyDB (m_hKeyDB));
   m_hKeyDB = kInvalidPGPKeyDBRef;

   // set path information if necessary
   char szPubDBPath [MAX_PATH];
   char szSecDBPath [MAX_PATH];
   RETURN_IF_ERROR (SetSpecialFilePath (szPubDBPath, pszPubDBFile, bUserLocalDir));
   RETURN_IF_ERROR (SetSpecialFilePath (szSecDBPath, pszSecDBFile, bUserLocalDir));

   // create file handles
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, szPubDBPath, &hPubFile));
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, szSecDBPath, &hSecFile));

   // set readonly / create option
   //PGPOpenKeyDBFileOptions opt = (bReadOnly ? kPGPOpenKeyDBFileOptions_None : kPGPOpenKeyDBFileOptions_Mutable | kPGPOpenKeyDBFileOptions_Create);
   int opt = (bReadOnly ? kPGPOpenKeyDBFileOptions_None : kPGPOpenKeyDBFileOptions_Mutable | kPGPOpenKeyDBFileOptions_Create);

   // open the keyring files
   PGPError err = PGPOpenKeyDBFile (m_hContext, (PGPOpenKeyDBFileOptions)opt, hPubFile, hSecFile, &m_hKeyDB);

   // cleanup
   if (PGPFileSpecRefIsValid (hPubFile)) PGPFreeFileSpec (hPubFile);
   if (PGPFileSpecRefIsValid (hSecFile)) PGPFreeFileSpec (hSecFile);

   RETURN_IF_ERROR (err);
   return 0;
   }


int CPKC::InitSeedFile ()
   {
   PGPFileSpecRef hEntropyFile = kInvalidPGPFileSpecRef;

   if (m_bSeedFileInit)
      return 0;

   char szEntropyPath [MAX_PATH];
   RETURN_IF_ERROR (SetSpecialFilePath (szEntropyPath, PGP_ENTROPY_FILE));
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, szEntropyPath, &hEntropyFile));
   RETURN_IF_ERROR (PGPSetRandSeedFile ((PFLFileSpecRef)hEntropyFile));

   // cleanup
   if (PGPFileSpecRefIsValid (hEntropyFile)) PGPFreeFileSpec (hEntropyFile);

   m_bSeedFileInit = TRUE;
   return 0;
   }



// internal.  used by openkeyring()
//
// if bUserLocalDir = false, the filenames are assumed to already contain whatever path information is needed
// if bUserLocalDir = true, a local application data path is prefixed on the filename
//
int CPKC::SetSpecialFilePath (PSZ pszDestPath, PSZ pszSrcFile, BOOL bUserLocalDir)
   {
   if (!bUserLocalDir)
      {
      strcpy (pszDestPath, pszSrcFile);
      return 0;
      }

   // get current user mydocs path
   // CSIDL_COMMON_APPDATA is 0x0023
   BOOL bOK = SHGetSpecialFolderPath (NULL, pszDestPath, 0x0023, 1) ||
              SHGetSpecialFolderPath (NULL, pszDestPath, CSIDL_APPDATA, 1);
   RETURN_IF_ASSERTERROR (bOK, PKCERR_NO_DIR);

   // add pgp specific subdir
   strcat (pszDestPath, "\\");
   strcat (pszDestPath, PGP_LOCAL_DATA_DIR);

//   // create the new directory if needed
//   if (_access (pszDestPath, 0))
//      mkdir (pszDestPath);
   
   // just try and create the dir.  I can no longer get
   // access() to compile.  I hate microsoft
   CreateDirectory (pszDestPath, NULL);

   strcat (pszDestPath, "\\");
   strcat (pszDestPath, pszSrcFile);
   return 0;
   }


// free's keyring db handle
//
int CPKC::CloseKeyRing ()
   {
   if (PGPKeyDBRefIsValid (m_hKeyDB))
      PGPFreeKeyDB (m_hKeyDB);
   m_hKeyDB = kInvalidPGPKeyDBRef;
   return 0;
   }


//////////////////////////////////////////////////////////////////////////////

// clears keyset to use for encrypting
//
int CPKC::ClearEncryptKeys ()
   {
   if (m_hEncryptKeyDB != kInvalidPGPKeyDBRef)
      PGPFreeKeyDB (m_hEncryptKeyDB);

   m_hEncryptKeyDB = kInvalidPGPKeyDBRef;
   return 0;
   }


// add a key to set to use for encrypting
// this fn adds a key that is in the local keyring
//
int CPKC::AddEncryptKeyFromKeyRing (PSZ pszID)
   {
   PGPKeySetRef hKeySet = kInvalidPGPKeySetRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszID && *pszID, PKCERR_NO_ID);

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // initialize encrypt key db if needed
   if (m_hEncryptKeyDB == kInvalidPGPKeyDBRef)
      RETURN_IF_ERROR (PGPNewKeyDB (m_hContext, &m_hEncryptKeyDB));

   // build keyset of matching keys
   RETURN_IF_ERROR (GetMatchingKeys (pszID, &hKeySet));

   // add keys to encrypt db
   RETURN_IF_ERROR (PGPCopyKeys (hKeySet, m_hEncryptKeyDB, NULL));

   // cleanup
   if (PGPKeySetRefIsValid (hKeySet)) PGPFreeKeySet (hKeySet);
   return 0;
   }


// add a key to set to use for encrypting
// this fn adds a key that is in an ascii buffer
//
int CPKC::AddEncryptKeyFromBuffer (PSZ pszPubKeyBuf)
   {
   PGPKeyDBRef  hKeyDB  = kInvalidPGPKeyDBRef;
   PGPKeySetRef hKeySet = kInvalidPGPKeySetRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszPubKeyBuf && *pszPubKeyBuf, PKCERR_NO_BUFFER);
 
   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // initialize encrypt key db if needed
   if (m_hEncryptKeyDB == kInvalidPGPKeyDBRef)
      RETURN_IF_ERROR (PGPNewKeyDB (m_hContext, &m_hEncryptKeyDB));

   // import file into the new DB
   PGPError err = PGPImport (m_hContext,  
                  &hKeyDB,
                  PGPOInputBuffer (m_hContext, pszPubKeyBuf, strlen (pszPubKeyBuf)),
                  PGPOLastOption (m_hContext));
   RETURN_IF_ERROR (err);

   // get new db keys into a keyset ...
   RETURN_IF_ERROR (PGPNewKeySet (hKeyDB, &hKeySet));

   // copy keys from new db to encrypt db
   RETURN_IF_ERROR (PGPCopyKeys (hKeySet, m_hEncryptKeyDB, NULL));

   // cleanup
   if (PGPKeySetRefIsValid (hKeySet)) PGPFreeKeySet (hKeySet);
   if (PGPKeyDBRefIsValid (hKeyDB))   PGPFreeKeyDB (hKeyDB);
   return 0;
   }


int CPKC::TestKeyFromBuffer (PSZ pszPubKeyBuf, CStringArray* pstrIDList)
   {
   PGPKeyDBRef  hKeyDB  = kInvalidPGPKeyDBRef;
   PGPKeySetRef hKeySet = kInvalidPGPKeySetRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszPubKeyBuf && *pszPubKeyBuf, PKCERR_NO_BUFFER);
 
   // import file into the new DB
   PGPError err = PGPImport (m_hContext,  
                  &hKeyDB,
                  PGPOInputBuffer (m_hContext, pszPubKeyBuf, strlen (pszPubKeyBuf)),
                  PGPOLastOption (m_hContext));
   RETURN_IF_ERROR (err);

   // get new db keys into a keyset ...
   RETURN_IF_ERROR (PGPNewKeySet (hKeyDB, &hKeySet));

   RETURN_IF_ERROR (GetKeySetIDList (hKeySet, pstrIDList))

   // cleanup
   if (PGPKeySetRefIsValid (hKeySet)) PGPFreeKeySet (hKeySet);
   if (PGPKeyDBRefIsValid (hKeyDB))   PGPFreeKeyDB (hKeyDB);
   return 0;
   }


// encrypts a file.  recipients are the encrypt keyset
//
int CPKC::EncryptFile (PSZ pszDestFile, PSZ pszSrcFile, BOOL bAsciiArmor)
   {
   PGPKeySetRef   hKeySet   = kInvalidPGPKeySetRef;
   PGPFileSpecRef hSrcFile  = kInvalidPGPFileSpecRef;
   PGPFileSpecRef hDestFile = kInvalidPGPFileSpecRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszDestFile && *pszDestFile && pszSrcFile && *pszSrcFile, PKCERR_NO_FILE);

   // check encrypt key set ...
   PGPUInt32 uKeys;
   RETURN_IF_ERROR (PGPCountKeysInKeyDB (m_hEncryptKeyDB, &uKeys));
   RETURN_IF_ASSERTERROR (!!uKeys, PKCERR_NO_KEY_MATCHES);

   // get encrypt to keys into a keyset ...
   RETURN_IF_ERROR (PGPNewKeySet (m_hEncryptKeyDB, &hKeySet));

   // make sure we have suffucuent entropy for the encode operation
   GuaranteeMinimumEntropy ();

   // make file references
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszSrcFile,  &hSrcFile));
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszDestFile, &hDestFile));

   // Encrypt the file
   PGPError err = PGPEncode (m_hContext,  
                     PGPOInputFile       (m_hContext, hSrcFile), 
                     PGPOOutputFile      (m_hContext, hDestFile),
                     PGPOArmorOutput     (m_hContext, bAsciiArmor),
                     PGPOEncryptToKeySet (m_hContext, hKeySet),
                     PGPOCommentString   (m_hContext, "Encrypted using Expedite Bid"),
                     PGPOLastOption      (m_hContext));
   // cleanup
   if (PGPKeySetRefIsValid (hKeySet))     PGPFreeKeySet (hKeySet);
   if (PGPFileSpecRefIsValid (hSrcFile))  PGPFreeFileSpec (hSrcFile);
   if (PGPFileSpecRefIsValid (hDestFile)) PGPFreeFileSpec (hDestFile);

   RETURN_IF_ERROR (err);
   return 0;
   }

//////////////////////////////////////////////////////////////////////////////


//static void printKeys( PGPKeySetRef keyset, const char *prefix, FILE *fout )
//{
//   PGPKeyListRef   klist;
//   PGPKeyIterRef   kiter;
//   PGPKeyDBObjRef   key;
//
//   PGPOrderKeySet( keyset, kPGPKeyOrdering_Any, FALSE, &klist );
//   PGPNewKeyIter( klist, &kiter );
//   while( IsntPGPError( PGPKeyIterNextKeyDBObj( kiter,
//                     kPGPKeyDBObjType_Key, &key ) ) ) {
//      char name[256];
//      PGPSize namelen = sizeof(name);
//      PGPGetPrimaryUserIDName( key, name, sizeof( name ), &namelen );
//      fprintf(fout, "%s%s\n", prefix, name);
//   }
//   PGPFreeKeyIter( kiter );
//   PGPFreeKeyList( klist );
//}






// internal, an event handler used by DecryptFile
//
int _DumpCallback (PGPContextRef hC, PGPEvent *event, PGPUserValue pUser)
   {
   PSZ p;

   switch (event->type)
      {
      case kPGPEvent_InitialEvent:    p="kPGPEvent_InitialEvent:   "; break;
      case kPGPEvent_FinalEvent:      p="kPGPEvent_FinalEvent:     "; break;
      case kPGPEvent_AnalyzeEvent:    p="kPGPEvent_AnalyzeEvent:   "; break;
      case kPGPEvent_BeginLexEvent:   p="kPGPEvent_BeginLexEvent:  "; break;
      case kPGPEvent_EndLexEvent:     p="kPGPEvent_EndLexEvent:    "; break;
      case kPGPEvent_DecryptionEvent: p="kPGPEvent_DecryptionEvent:"; break;
      case kPGPEvent_PassphraseEvent: p="kPGPEvent_PassphraseEvent:"; break;
      case kPGPEvent_OutputEvent:     p="kPGPEvent_OutputEvent:    "; break;
      case kPGPEvent_SignatureEvent:  p="kPGPEvent_SignatureEvent: "; break;
      case kPGPEvent_RecipientsEvent: p="kPGPEvent_RecipientsEvent:"; break;
      case kPGPEvent_KeyFoundEvent:   p="kPGPEvent_KeyFoundEvent:"  ; break;
      case kPGPEvent_ErrorEvent:      p="kPGPEvent_ErrorEvent:   "  ; break;
      case kPGPEvent_WarningEvent:    p="kPGPEvent_WarningEvent: "  ; break;
      default:                        p="Unknown"                   ; break;
      }
   printf ("Event: %s [%d]\n", p, event->type);
   return 0;
   }




// internal, an event handler used by DecryptFile
//
int _DecryptCallback (PGPContextRef hContext, PGPEvent* pEvent, PGPUserValue pUserValue)
   {
   CPKC* pCPKC = (CPKC*)pUserValue;

   switch (pEvent->type)
      {
      // called first.  we do state init here
      case kPGPEvent_InitialEvent:
         {
         pCPKC->m_bGotDecryptionEvent = 0;
         pCPKC->m_bGotSignatureEvent  = 0;
         pCPKC->m_bGotPassphraseEvent = 0;
         *(pCPKC->m_szSignKeyID)    = '\0';
          break;
         }
      // called if the data is a signed block
      case kPGPEvent_SignatureEvent:
         {
         PGPEventSignatureData* pSigData = &pEvent->data.signatureData;
         PGPSize len = sizeof(pCPKC->m_szSignKeyID);

         if (pSigData->signingKey)
            PGPGetPrimaryUserIDName (pSigData->signingKey, pCPKC->m_szSignKeyID, sizeof(pCPKC->m_szSignKeyID), &len);
         else
            strcpy (pCPKC->m_szSignKeyID, "Unknown");
         pCPKC->m_bGotSignatureEvent = 1;
         break;
         }
      // called if we are ok and are decrypting an encrypted block
      case kPGPEvent_DecryptionEvent:
           {
         pCPKC->m_bGotDecryptionEvent = 1;
          break;
         }
      // called if we do not have a valid userid or password
      case kPGPEvent_PassphraseEvent:
           {
         pCPKC->m_bGotPassphraseEvent = 1;
          break;
         }
      }
   return 0;
   }




// decrypts a file
// a decryption key must be on the local keyring
//
int CPKC::DecryptFile (PSZ pszDestFile, PSZ pszSrcFile, PSZ pszPWD)
   {
   PGPFileSpecRef hSrcFile  = kInvalidPGPFileSpecRef;
   PGPFileSpecRef hDestFile = kInvalidPGPFileSpecRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszDestFile && *pszDestFile && pszSrcFile && *pszSrcFile, PKCERR_NO_FILE);

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // make file references
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszSrcFile,  &hSrcFile));
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszDestFile, &hDestFile));

   // Decrypt the file
   PGPError err = PGPDecode ( m_hContext,  
                     PGPOInputFile  (m_hContext, hSrcFile), 
                     PGPOOutputFile (m_hContext, hDestFile),
                     PGPOKeyDBRef   (m_hContext, m_hKeyDB),
                     PGPOPassphrase (m_hContext, pszPWD),
                     PGPOEventHandler (m_hContext, _DecryptCallback, (PGPUserValue)(void*)this),
                     PGPOLastOption (m_hContext));
   // cleanup
   if (PGPFileSpecRefIsValid (hSrcFile))  PGPFreeFileSpec (hSrcFile);
   if (PGPFileSpecRefIsValid (hDestFile)) PGPFreeFileSpec (hDestFile);

   RETURN_IF_ERROR (err);

   RETURN_IF_ASSERTERROR (!m_bGotPassphraseEvent, PKCERR_BAD_PASSWORD      );
   RETURN_IF_ASSERTERROR (!m_bGotSignatureEvent,  PKCERR_BLOCK_IS_SIGNATURE);
   RETURN_IF_ASSERTERROR (m_bGotDecryptionEvent,  PKCERR_DID_NOT_DECRYPT   );

   return 0;
   }


//////////////////////////////////////////////////////////////////////////////

// signs a file
// the sign key must be on the local keyring
//
int CPKC::SignFile (PSZ pszDestFile, PSZ pszSrcFile, PSZ pszSignID, PSZ pszSignPWD, BOOL bSignInClear)
   {
   PGPFileSpecRef hSrcFile     = kInvalidPGPFileSpecRef;
   PGPFileSpecRef hDestFile    = kInvalidPGPFileSpecRef;
   PGPKeyDBObjRef hKey         = kInvalidPGPKeyDBObjRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszDestFile && *pszDestFile && pszSrcFile && *pszSrcFile, PKCERR_NO_FILE);
   RETURN_IF_ASSERTERROR (pszSignID && *pszSignID, PKCERR_NO_ID);
   RETURN_IF_ASSERTERROR (pszSignPWD && *pszSignPWD, PKCERR_NO_PWD);

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // make sure we have suffucuent entropy for the encode operation
   GuaranteeMinimumEntropy ();

   // make file references
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszSrcFile,  &hSrcFile));
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszDestFile, &hDestFile));

   RETURN_IF_ERROR (GetMatchingKey (pszSignID, &hKey, TRUE));

   // build clear sign option
   PGPOptionList* hClearOption = (bSignInClear ? PGPOClearSign (m_hContext, TRUE) : PGPONullOption (m_hContext));

   // Sign the file
   PGPError err = PGPEncode ( m_hContext,  
                     hClearOption,
                     PGPOArmorOutput (m_hContext, TRUE),          
                     PGPOInputFile   (m_hContext, hSrcFile), 
                     PGPOOutputFile  (m_hContext, hDestFile),
                     PGPOSignWithKey (m_hContext, hKey,
                        PGPOPassphrase (m_hContext, pszSignPWD),
                        PGPOLastOption (m_hContext)),
                     PGPOCommentString (m_hContext, "Signed using Expedite Bid"),
                     PGPOLastOption (m_hContext));
   // cleanup 
   if (PGPFileSpecRefIsValid (hSrcFile))    PGPFreeFileSpec (hSrcFile);
   if (PGPFileSpecRefIsValid (hDestFile))   PGPFreeFileSpec (hDestFile);

   RETURN_IF_ERROR(err);
   return 0;
   }


int CPKC::ExtractSignature (PSZ pszDestFile, PSZ pszSrcFile, PSZ pszSignKeyID)
   {
   PGPFileSpecRef hSrcFile  = kInvalidPGPFileSpecRef;
   PGPFileSpecRef hDestFile = kInvalidPGPFileSpecRef;

   if (pszSignKeyID)
         *pszSignKeyID = '\0';

   // param sanity check
   RETURN_IF_ASSERTERROR (pszDestFile && *pszDestFile && pszSrcFile && *pszSrcFile, PKCERR_NO_FILE);

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // make file references
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszSrcFile,  &hSrcFile));
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszDestFile, &hDestFile));


   PGPError err = PGPDecode (m_hContext,  
                     PGPOInputFile   (m_hContext, hSrcFile), 
                     PGPOOutputFile  (m_hContext, hDestFile),
                     PGPOKeyDBRef    (m_hContext, m_hKeyDB),
                     PGPOEventHandler(m_hContext, _DecryptCallback, (PGPUserValue)(void*)this),
                     PGPOLastOption  (m_hContext));

   // cleanup
   if (PGPFileSpecRefIsValid (hSrcFile))  PGPFreeFileSpec (hSrcFile);
   if (PGPFileSpecRefIsValid (hDestFile)) PGPFreeFileSpec (hDestFile);

   RETURN_IF_ERROR (err);
   RETURN_IF_ASSERTERROR (m_bGotSignatureEvent, PKCERR_BLOCK_IS_NOT_SIGNATURE);

   if (pszSignKeyID)
      strcpy (pszSignKeyID, m_szSignKeyID);

   return 0;
   }



//////////////////////////////////////////////////////////////////////////////

// internal, an event handler used by CreateKey()
//
int _CreateKeyCallback (PGPContextRef hC, PGPEvent *event, PGPUserValue pUser)
   {
   EVENTFN pfnEvent = (EVENTFN) pUser;

   if (event->type == kPGPEvent_KeyGenEvent && pfnEvent)
      pfnEvent (2, 0);
   return 0;
   }


// Creates a key
//
// The caller should:
//    -Make sure there is enough entropy to make a key.  This fn will create
//     the entropy if needed, but the user should really handle it so that there
//     can be feedback for the user.
//    -Pass an event to pfnEvent so that there can be feedback for the user.
//
int CPKC::CreateKey (PSZ pszID, PSZ pszPWD, EVENTFN pfnEvent)
   {
   PGPError err;
    PGPKeyDBObjRef   hKey    = kInvalidPGPKeyDBObjRef;
   PGPKeyDBObjRef   hSubKey = kInvalidPGPKeyDBObjRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszID  && *pszID , PKCERR_NO_ID);
   RETURN_IF_ASSERTERROR (pszPWD && *pszPWD, PKCERR_NO_PWD);

   // make sure there is enough entropy for this operation
   GuaranteeMinimumEntropy ();

   err = PGPGenerateKey (m_hContext, 
            &hKey,
            PGPOKeyGenParams    (m_hContext, kPGPPublicKeyAlgorithm_DSA, KEYSIZE),
            PGPOKeyGenName      (m_hContext, pszID, strlen(pszID)),
            PGPOExpiration      (m_hContext, kPGPExpirationTime_Never),
            PGPOKeyDBRef        (m_hContext, m_hKeyDB),
            PGPOPassphrase      (m_hContext, pszPWD),
            PGPOKeyGenFast      (m_hContext, TRUE),
            PGPOKeyFlags        (m_hContext, kPGPKeyPropertyFlags_UsageSign),
            PGPOEventHandler    (m_hContext, _CreateKeyCallback, (PGPUserValue)pfnEvent),
            PGPOLastOption      (m_hContext));
   RETURN_IF_ERROR(err);

   // Generate a SubKey
   err = PGPGenerateSubKey (m_hContext, 
            &hSubKey,
            PGPOKeyGenMasterKey (m_hContext, hKey),
            PGPOKeyGenParams    (m_hContext, kPGPPublicKeyAlgorithm_ElGamal, KEYSIZE),
            PGPOExpiration      (m_hContext, kPGPExpirationTime_Never),
            PGPOPassphrase      (m_hContext, pszPWD),
            PGPOKeyGenFast      (m_hContext, TRUE),
            PGPOKeyFlags        (m_hContext, kPGPKeyPropertyFlags_UsageEncrypt),
            PGPOEventHandler    (m_hContext, _CreateKeyCallback, (PGPUserValue)pfnEvent),
            PGPOLastOption      (m_hContext));
   RETURN_IF_ERROR(err);

   // commit changes
   RETURN_IF_ERROR(PGPFlushKeyDB (m_hKeyDB));
   return 0;
   }


// deletes a key from the local keyring
//
int CPKC::DeleteKey (PSZ pszID)
   {
   PGPKeySetRef hKeySet = kInvalidPGPKeySetRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszID && *pszID, PKCERR_NO_ID);

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // build keyset of matching keys
   RETURN_IF_ERROR (GetMatchingKeys (pszID, &hKeySet));

   // delete it (them)
   RETURN_IF_ERROR (PGPDeleteKeys (hKeySet));

   // cleanup
   if (PGPKeySetRefIsValid (hKeySet)) PGPFreeKeySet (hKeySet);
   return 0;
   }


// exports a key from the local keyring
//
int CPKC::ExportKey (PSZ pszKeyID, PSZ pszKeyFile, BOOL bInclPrivate, PSZ pszExportMsg)
   {
   PGPError err;
   PGPKeySetRef   hKeySet     = kInvalidPGPKeySetRef;
   PGPFileSpecRef hExportFile = kInvalidPGPFileSpecRef;
   char szFingerprint[64];
   char szNewExportMessage[256];

   // param sanity check
   RETURN_IF_ASSERTERROR (pszKeyID && *pszKeyID, PKCERR_NO_ID);
   RETURN_IF_ASSERTERROR (pszKeyFile && *pszKeyFile, PKCERR_NO_FILE);

   pszExportMsg = (pszExportMsg ? pszExportMsg : "Exported by Expedite Bid");
   
   GetKeyFingerprint (pszKeyID, szFingerprint);
   sprintf(szNewExportMessage,"%s - Fingerprint: %s",pszExportMsg,szFingerprint);
   

   PGPOptionList* hCommentOption = (szNewExportMessage ? PGPOCommentString (m_hContext, szNewExportMessage) : PGPONullOption (m_hContext));

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // build keyset of matching keys
   RETURN_IF_ERROR (GetMatchingKeys (pszKeyID, &hKeySet));

   // create file handles
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszKeyFile, &hExportFile));

   // Export the key(s)
   err = PGPExport (m_hContext, 
            PGPOExportKeySet      (m_hContext, hKeySet),
            PGPOExportPrivateKeys (m_hContext, bInclPrivate),
            PGPOArmorOutput       (m_hContext, TRUE),
            PGPOOutputFile        (m_hContext, hExportFile),
            hCommentOption,
            PGPOLastOption        (m_hContext));
   // cleanup
   if (PGPFileSpecRefIsValid (hExportFile)) PGPFreeFileSpec (hExportFile);
   if (PGPKeySetRefIsValid (hKeySet))       PGPFreeKeySet (hKeySet);

   RETURN_IF_ERROR(err);
   return 0;
   }


////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//// internal, an event handler 
//
//int _ImportCallback (PGPContextRef hContext, PGPEvent* pEvent, PGPUserValue pUserValue)
//   {
//   CPKC* pCPKC = (CPKC*)pUserValue;
//   PSZ p;
//
//   switch (pEvent->type)
//      {
//      case kPGPEvent_NullEvent             : p="kPGPEvent_NullEvent             "; break;
//      case kPGPEvent_InitialEvent          : p="kPGPEvent_InitialEvent          "; break;
//      case kPGPEvent_FinalEvent            : p="kPGPEvent_FinalEvent            "; break;
//      case kPGPEvent_ErrorEvent            : p="kPGPEvent_ErrorEvent            "; break;
//      case kPGPEvent_WarningEvent          : p="kPGPEvent_WarningEvent          "; break;
//      case kPGPEvent_EntropyEvent          : p="kPGPEvent_EntropyEvent          "; break;
//      case kPGPEvent_PassphraseEvent       : p="kPGPEvent_PassphraseEvent       "; break;
//      case kPGPEvent_InsertKeyEvent        : p="kPGPEvent_InsertKeyEvent        "; break;
//      case kPGPEvent_AnalyzeEvent          : p="kPGPEvent_AnalyzeEvent          "; break;
//      case kPGPEvent_RecipientsEvent       : p="kPGPEvent_RecipientsEvent       "; break;
//      case kPGPEvent_KeyFoundEvent         : p="kPGPEvent_KeyFoundEvent         "; break;
//      case kPGPEvent_OutputEvent           : p="kPGPEvent_OutputEvent           "; break;
//      case kPGPEvent_SignatureEvent        : p="kPGPEvent_SignatureEvent        "; break;
//      case kPGPEvent_BeginLexEvent         : p="kPGPEvent_BeginLexEvent         "; break;
//      case kPGPEvent_EndLexEvent           : p="kPGPEvent_EndLexEvent           "; break;
//      case kPGPEvent_RecursionEvent        : p="kPGPEvent_RecursionEvent        "; break;
//      case kPGPEvent_DetachedSignatureEvent: p="kPGPEvent_DetachedSignatureEvent"; break;
//      case kPGPEvent_KeyGenEvent           : p="kPGPEvent_KeyGenEvent           "; break;
//      case kPGPEvent_KeyServerEvent        : p="kPGPEvent_KeyServerEvent        "; break;
//      case kPGPEvent_KeyServerSignEvent    : p="kPGPEvent_KeyServerSignEvent    "; break;
//      case kPGPEvent_KeyServerTLSEvent     : p="kPGPEvent_KeyServerTLSEvent     "; break;
//      case kPGPEvent_KeyServerIdleEvent    : p="kPGPEvent_KeyServerIdleEvent    "; break;
//      case kPGPEvent_SocketsIdleEvent      : p="kPGPEvent_SocketsIdleEvent      "; break;
//      case kPGPEvent_DecryptionEvent       : p="kPGPEvent_DecryptionEvent       "; break;
//      case kPGPEvent_EncryptionEvent       : p="kPGPEvent_EncryptionEvent       "; break;
//      case kPGPEvent_ToBeSignedEvent       : p="kPGPEvent_ToBeSignedEvent       "; break;
//      default:                               p="Unknown"                         ; break;
//      }
//   printf ("Event: %s [%d]\n", p, pEvent->type);
//   return 0;
//   }
//


// imports a key from a file into the local keyring
// if the key is password protected, pass the pszPWD parameter
// pszKeyIDName - output - the key we just imported
int CPKC::ImportKey (PSZ pszKeyFile, int iFormatFlag, PSZ pszPWD, PSZ pszImportedKeyIDName)
   {
   PGPFileSpecRef hImportFile = kInvalidPGPFileSpecRef;
   PGPKeyDBRef    hKeyDB      = kInvalidPGPKeyDBRef;
   PGPKeySetRef   hKeySet     = kInvalidPGPKeySetRef;
   PGPError       err;

   // param sanity check
   RETURN_IF_ASSERTERROR (pszKeyFile && *pszKeyFile, PKCERR_NO_FILE);

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // create file handles
   RETURN_IF_ERROR (PGPNewFileSpecFromFullPath (m_hContext, pszKeyFile, &hImportFile));

   // build conditional options
   PGPOptionList*   hPasswordOption = (pszPWD ? PGPOPassphrase (m_hContext, pszPWD): PGPONullOption (m_hContext));

   err = PGPImport (m_hContext,
                    &hKeyDB,
                    hPasswordOption,
                    InputFormatOption (iFormatFlag),
                    PGPOInputFile  (m_hContext, hImportFile),
                    PGPOKeyDBRef   (m_hContext, m_hKeyDB),
//                  PGPOEventHandler (m_hContext, _ImportCallback, (PGPUserValue)(void*)this),
                    PGPOLastOption (m_hContext));

   // get new db keys into a keyset ...
   if (!err) err = PGPNewKeySet (hKeyDB, &hKeySet);


   if (iFormatFlag == PGPIMPORT_FORMAT_PKCS12)
      {
      PGPKeyDBObjRef hKey      = kInvalidPGPKeyDBObjRef;
      PGPKeyDBObjRef hUserID   = kInvalidPGPKeyDBObjRef;
      PGPKeyIterRef  hIterator = kInvalidPGPKeyIterRef;

      // Get the key and it's user ID
      if (!err) err = PGPNewKeyIterFromKeySet (hKeySet, &hIterator);
      if (!err) err = PGPKeyIterNextKeyDBObj (hIterator, kPGPKeyDBObjType_Key, &hKey);
    	if (!err) err = PGPGetPrimaryUserID(hKey, &hUserID);

      if (PGPKeyIterRefIsValid (hIterator)) PGPFreeKeyIter (hIterator);

      // make sure we have suffucuent entropy for the encode operation
      GuaranteeMinimumEntropy ();

      if (!err) err = PGPCertifyUserID (hUserID, hKey, 
                                        PGPOPassphrase (m_hContext, pszPWD),
			                               PGPOExpiration (m_hContext, 0),
			                               PGPOExportable (m_hContext, TRUE),
			                               PGPOSigTrust   (m_hContext, 2,  kPGPKeyTrust_Complete),
                                        PGPOLastOption (m_hContext));
      }

   // copy keys from new db to master db
   if (!err) err = PGPCopyKeys (hKeySet, m_hKeyDB, NULL);

   if (pszImportedKeyIDName) // does the caller want the name of the key we imported ?
      {
      PGPKeyDBObjRef hKey      = kInvalidPGPKeyDBObjRef;
      PGPKeyIterRef  hIterator = kInvalidPGPKeyIterRef;
      PGPSize        iLength   = 256;

      *pszImportedKeyIDName = '\0';
      if (!err) err = PGPNewKeyIterFromKeySet (hKeySet, &hIterator);
      if (!err) err = PGPKeyIterNextKeyDBObj (hIterator, kPGPKeyDBObjType_Key, &hKey);
      if (!err) err = PGPGetPrimaryUserIDName (hKey, pszImportedKeyIDName, iLength, &iLength);

      if (PGPKeyIterRefIsValid (hIterator)) PGPFreeKeyIter (hIterator);
      }

   // cleanup
   if (PGPKeySetRefIsValid   (hKeySet)    ) PGPFreeKeySet   (hKeySet);
   if (PGPKeyDBRefIsValid    (hKeyDB)     ) PGPFreeKeyDB    (hKeyDB);
   if (PGPFileSpecRefIsValid (hImportFile)) PGPFreeFileSpec (hImportFile);

   RETURN_IF_ERROR(err);
   return 0;
   }


// imports a key from a buffer into the local keyring
// if the key is password protected, pass the pszPWD parameter
//
int CPKC::ImportKey (BYTE* pbKeyBuffer, int iKeyBufferLen, int iFormatFlag, PSZ pszPWD)
   {
   PGPError err;
   PGPKeyDBRef  hKeyDB  = kInvalidPGPKeyDBRef;
   PGPKeySetRef hKeySet = kInvalidPGPKeySetRef;

   // param sanity check
   RETURN_IF_ASSERTERROR (pbKeyBuffer && iKeyBufferLen, PKCERR_NO_FILE);

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // build conditional options
   PGPOptionList*  hPasswordOption = (pszPWD ? PGPOPassphrase (m_hContext, pszPWD): PGPONullOption (m_hContext));

   err = PGPImport (m_hContext,
                    &hKeyDB,
                    hPasswordOption,
                    InputFormatOption (iFormatFlag),
                    PGPOInputBuffer (m_hContext, pbKeyBuffer, iKeyBufferLen),
                    PGPOKeyDBRef   (m_hContext, m_hKeyDB),
//                    PGPOEventHandler (m_hContext, _ImportCallback, (PGPUserValue)(void*)this),
                    PGPOLastOption (m_hContext));

   // get new db keys into a keyset ...
   if (!err) err = PGPNewKeySet (hKeyDB, &hKeySet);

   // copy keys from new db to master db
   if (!err) err = PGPCopyKeys (hKeySet, m_hKeyDB, NULL);

   // cleanup
   if (PGPKeySetRefIsValid (hKeySet)) PGPFreeKeySet (hKeySet);
   if (PGPKeyDBRefIsValid (hKeyDB))   PGPFreeKeyDB  (hKeyDB);

   RETURN_IF_ERROR(err);
   return 0;
   }

////////////////////////////////////////////////////////////////////////////////////////



// internal, used by ImportKeyFile ()
//
PGPOptionList* CPKC::InputFormatOption (int iFormatFlag)
   {
   enum PGPInputFormat_ hFormat;

   switch (iFormatFlag)
      {
      case PGPIMPORT_FORMAT_COMPATIBLE: hFormat = kPGPInputFormat_PGP;                 break;
      case PGPIMPORT_FORMAT_PEM:        hFormat = kPGPInputFormat_PEMEncodedX509Cert;  break;
      case PGPIMPORT_FORMAT_PKCS7:      hFormat = kPGPInputFormat_X509DataInPKCS7;     break;
      case PGPIMPORT_FORMAT_PKCS8:      hFormat = kPGPInputFormat_PrivateKeyInfo;      break;
      case PGPIMPORT_FORMAT_PKCS12:     hFormat = kPGPInputFormat_PKCS12;              break;
      default:  return PGPONullOption (m_hContext);
      }
   return PGPOInputFormat (m_hContext, hFormat);
   }


//////////////////////////////////////////////////////////////////////////////


void CPKC::BoolProperty (int iIndent, PGPKeyDBObjRef hKey, PGPKeyDBObjProperty iProperty, PSZ pszStr)
   {
   PGPBoolean bProp;
   PGPError err;
   
   SetError (err = PGPGetKeyDBObjBooleanProperty (hKey, iProperty, &bProp));
   printf ("%*s%s%*s: %s\n", iIndent, " ", pszStr, 
                             23 - strlen (pszStr), "", err ? GetError () : (bProp ? "TRUE" : "FALSE"));
   }


void CPKC::NumProperty (int iIndent, PGPKeyDBObjRef hKey, PGPKeyDBObjProperty iProperty, PSZ pszStr)
   {
   PGPInt32 iVal = 0;
   PGPError err;
 
   if (SetError (err = PGPGetKeyDBObjNumericProperty (hKey, iProperty, &iVal)))
                             
      printf ("%*s%s%*s: %s\n", iIndent, "", pszStr, 23 - strlen (pszStr),  "", GetError ());
   else
      printf ("%*s%s%*s: %d\n", iIndent, "", pszStr, 23 - strlen (pszStr),  "", iVal);
   }


int CPKC::_keyInfo (int iIndent, PGPKeyDBObjRef hKey, BOOL bPrimary)
   {
   char szKeyName [256];


   printf ("-------------------------------------------------------------------------------\n");
   if (bPrimary)
      {
      PGPSize iLength = sizeof (szKeyName);
      PGPGetPrimaryUserIDName (hKey, szKeyName, sizeof (szKeyName), &iLength);
      printf ("Key properties for [%s]:\n", szKeyName);
      }

   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsSecret                   , "IsSecret"               );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsAxiomatic                , "IsAxiomatic"            );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsRevoked                  , "IsRevoked"              );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsDisabled                 , "IsDisabled"             );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsNotCorrupt               , "IsNotCorrupt"           );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsExpired                  , "IsExpired"              );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_NeedsPassphrase            , "NeedsPassphrase"        );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_HasUnverifiedRevocation    , "HasUnverifiedRevocation");
   BoolProperty (iIndent, hKey, kPGPKeyProperty_CanEncrypt                 , "CanEncrypt"             );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_CanDecrypt                 , "CanDecrypt"             );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_CanSign                    , "CanSign"                );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_CanVerify                  , "CanVerify"              );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsEncryptionKey            , "IsEncryptionKey"        );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsSigningKey               , "IsSigningKey"           );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsSecretShared             , "IsSecretShared"         );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsRevocable                , "IsRevocable"            );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_HasThirdPartyRevocation    , "HasThirdPartyRevocation");
   BoolProperty (iIndent, hKey, kPGPKeyProperty_HasCRL                     , "HasCRL"                 );
   BoolProperty (iIndent, hKey, kPGPKeyProperty_IsOnToken                  , "IsOnToken"              );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_AlgorithmID                , "AlgorithmID "           );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_Bits                       , "Bits"                   );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_Trust                      , "Trust"                  );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_Validity                   , "Validity"               );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_Flags                      , "Flags"                  );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_HashAlgorithmID            , "HashAlgorithmID"        );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_Version                    , "Version"                );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_TokenNum                   , "TokenNum"               );
   NumProperty  (iIndent, hKey, kPGPKeyProperty_Features                   , "Features"               );
   printf ("-------------------------------------------------------------------------------\n");
   return 0;
   }

int CPKC::_subkeyInfo (int iIndent, PGPKeyDBObjRef hKey)
   {
   printf ("-------------------------------------------------------------------------------\n");
   BoolProperty (iIndent, hKey, kPGPSubKeyProperty_IsRevoked               , "IsRevoked"              );
   BoolProperty (iIndent, hKey, kPGPSubKeyProperty_IsNotCorrupt            , "IsNotCorrupt"           );
   BoolProperty (iIndent, hKey, kPGPSubKeyProperty_IsExpired               , "IsExpired"              );
//   BoolProperty (iIndent, hKey, kPGPSubKeyPropertrty_NeedsPassphrase       , "NeedsPassphrase"        );
   BoolProperty (iIndent, hKey, kPGPSubKeyProperty_HasUnverifiedRevocation , "HasUnverifiedRevocation");
   BoolProperty (iIndent, hKey, kPGPSubKeyProperty_IsRevocable             , "IsRevocable"            );
   BoolProperty (iIndent, hKey, kPGPSubKeyProperty_HasThirdPartyRevocation , "HasThirdPartyRevocation");
   BoolProperty (iIndent, hKey, kPGPSubKeyProperty_IsOnToken               , "IsOnToken"              );
   NumProperty  (iIndent, hKey, kPGPSubKeyProperty_AlgorithmID             , "AlgorithmID"            );
   NumProperty  (iIndent, hKey, kPGPSubKeyProperty_Bits                    , "Bits"                   );
   NumProperty  (iIndent, hKey, kPGPSubKeyProperty_Version                 , "Version"                );
   NumProperty  (iIndent, hKey, kPGPSubKeyProperty_Flags                   , "Flags"                  );
   printf ("-------------------------------------------------------------------------------\n");
   return 0;
   }



int CPKC::KeyInfo (PSZ pszID, BOOL bExact)
   {
   PGPKeySetRef   hKeySet   = kInvalidPGPKeySetRef;
   PGPKeyListRef  hKeyList  = kInvalidPGPKeyListRef;
   PGPKeyDBObjRef hKey      = kInvalidPGPKeyDBObjRef;
   PGPKeyDBObjRef hSubKey   = kInvalidPGPKeyDBObjRef;
   PGPKeyIterRef  hIterator = kInvalidPGPKeyIterRef;

   RETURN_IF_ERROR (GetMatchingKeys (pszID, &hKeySet));
   RETURN_IF_ERROR (PGPOrderKeySet (hKeySet, kPGPKeyOrdering_UserID, FALSE, &hKeyList));

   RETURN_IF_ERROR (PGPNewKeyIter (hKeyList, &hIterator));
   while (!PGPKeyIterNextKeyDBObj (hIterator, kPGPKeyDBObjType_Key, &hKey)) 
      {
      _keyInfo (3, hKey, TRUE);

      while (!PGPKeyIterNextKeyDBObj (hIterator, kPGPKeyDBObjType_SubKey, &hSubKey)) 
         {
         _keyInfo (6, hSubKey, FALSE);
         _subkeyInfo (6, hSubKey);
         }
      }


   if (PGPKeyIterRefIsValid (hIterator)) PGPFreeKeyIter (hIterator);
   if (PGPKeyListRefIsValid (hKeyList))  PGPFreeKeyList (hKeyList);
   if (PGPKeySetRefIsValid  (hKeySet))   PGPFreeKeySet  (hKeySet);
   return 0;
   }




// tells how much entropy is available
//
int CPKC::GetEntropyAvailable ()
   {
   return PGPGlobalRandomPoolGetEntropy ();
   }

// tells how much entropy is needed to create a keypair
//
int CPKC::GetEntropyNeeded () 
   {
   InitSeedFile ();

   return PGPGetKeyEntropyNeeded(m_hContext,
              PGPOKeyGenParams(m_hContext, kPGPPublicKeyAlgorithm_DSA, KEYSIZE),
              PGPOKeyGenFast  (m_hContext, TRUE),
              PGPOLastOption  (m_hContext))
        + PGPGetKeyEntropyNeeded(m_hContext,
             PGPOKeyGenParams(m_hContext, kPGPPublicKeyAlgorithm_ElGamal, KEYSIZE),
             PGPOKeyGenFast  (m_hContext, TRUE),
             PGPOLastOption  (m_hContext));
   }


// Adds entropy.  Call with one of: PGP_EN_MOUSE | PGP_EN_SYSTEM
//
int CPKC::AddEntropy (int iEntropyTypeFlag)
   {
   PGPError err;

   if (iEntropyTypeFlag & PGP_EN_MOUSE )
      err = PGPGlobalRandomPoolMouseMoved ();
   if (iEntropyTypeFlag & PGP_EN_SYSTEM)
      {
//    assert (0); // The SystemState() fn seems to be unsupported!
//    err = PGPGlobalRandomPoolAddSystemState ();
      err = PGPGlobalRandomPoolMouseMoved ();
      }
   return 0; 
   }


int CPKC::GuaranteeMinimumEntropy ()
   {
   while (!PGPGlobalRandomPoolHasMinimumEntropy())
      AddEntropy (PGP_EN_SYSTEM);
   return 0;
   }

//////////////////////////////////////////////////////////////////////////////


// determines if a given key exists in the local keyring
// note: this function is case sensitive, so BIDDER will not match bidder
int CPKC::KeyExists (PSZ pszID, BOOL* pbExists)
   {
   int iKeyCount;
   RETURN_IF_ERROR (GetKeyMatchCount (pszID, &iKeyCount, TRUE));
   *pbExists = !!iKeyCount;
   return 0;
   }


// counts how many keys in the local keyring match a given string
//
int CPKC::GetKeyMatchCount (PSZ pszIDMatchSpec, int* piMatchCount, BOOL bExact)
   {
   PGPUInt32    iKeyCount;
   PGPKeySetRef hKeySet = kInvalidPGPKeySetRef;

   *piMatchCount = 0;

   // get matching keys and count em
   RETURN_IF_ERROR (GetMatchingKeys (pszIDMatchSpec, &hKeySet, bExact));
   RETURN_IF_ERROR (PGPCountKeys (hKeySet, &iKeyCount));
   *piMatchCount = (int)iKeyCount;

   // cleanup
   if (PGPKeySetRefIsValid (hKeySet)) PGPFreeKeySet (hKeySet);
   return 0;
   }


int CPKC::GetKeySetIDList (PGPKeySetRef hKeySet, CStringArray* pstrIDList)
   {
   char           szKeyName [256];
   PGPKeyListRef  hKeyList   = kInvalidPGPKeyListRef;
   PGPKeyIterRef  hIterator  = kInvalidPGPKeyIterRef;
   PGPKeyDBObjRef hKey       = kInvalidPGPKeyDBObjRef;

   pstrIDList->RemoveAll();

   // order them by ID
   RETURN_IF_ERROR (PGPOrderKeySet (hKeySet, kPGPKeyOrdering_UserID, FALSE, &hKeyList));

   // loop through em
   RETURN_IF_ERROR (PGPNewKeyIter (hKeyList, &hIterator));
   while (!PGPKeyIterNextKeyDBObj (hIterator, kPGPKeyDBObjType_Key, &hKey)) 
      {
      PGPSize iLength = sizeof (szKeyName);
      PGPGetPrimaryUserIDName (hKey, szKeyName, sizeof (szKeyName), &iLength);
      pstrIDList->Add (szKeyName);
      }

   // cleanup
   if (PGPKeyIterRefIsValid (hIterator)) PGPFreeKeyIter (hIterator);
   if (PGPKeyListRefIsValid (hKeyList))  PGPFreeKeyList (hKeyList);
   return 0;
   }


// returns a list of UserID's of keys matching a given UserID
//
int CPKC::GetKeyMatchList (PSZ pszIDMatchSpec, CStringArray* pstrMatchList)
   {
   PGPKeySetRef hKeySet = kInvalidPGPKeySetRef;

   RETURN_IF_ERROR (GetMatchingKeys (pszIDMatchSpec, &hKeySet));
   RETURN_IF_ERROR (GetKeySetIDList (hKeySet, pstrMatchList));

   // cleanup
   if (PGPKeySetRefIsValid (hKeySet)) PGPFreeKeySet  (hKeySet);
   return 0;
   }


int CPKC::SetKeyNotation (PSZ pszID, PSZ pszPWD, PSZ pszNotation)
   {
   PGPKeyDBObjRef hKey = kInvalidPGPKeyDBObjRef;
   RETURN_IF_ERROR (GetMatchingKey (pszID, &hKey, FALSE));
   
   RETURN_IF_ERROR (PGPAddAttributeUserID (hKey, 
                                           kPGPAttribute_Notation, 
                                           (unsigned char *)pszNotation,
                                           strlen (pszNotation)+1, 
                                           PGPOPassphrase (m_hContext, pszPWD), 
                                       PGPOLastOption (m_hContext)));
   return 0;
   }


int CPKC::GetKeyNotation (PSZ pszID, PSZ pszNotation)
   {
   PGPError err;

   *pszNotation = '\0';

   PGPKeyDBObjRef hKey = kInvalidPGPKeyDBObjRef;
   err = GetMatchingKey (pszID, &hKey, FALSE);
   if (err == kPGPError_EndOfIteration)
      return 0; // no matching key
   RETURN_IF_ERROR (err);

   PGPKeyDBObjRef hUserID = kInvalidPGPKeyDBObjRef;
   if (PGPGetPrimaryAttributeUserID (hKey, kPGPAttribute_Notation, &hUserID))
      return 0; // none present
   
   int iAttributeType;
   RETURN_IF_ERROR (PGPGetKeyDBObjNumericProperty (hUserID, kPGPUserIDProperty_AttributeType, &iAttributeType));
   if (iAttributeType != kPGPAttribute_Notation)
      return 0; // none present

   unsigned int iNoteSize;
   RETURN_IF_ERROR (PGPGetKeyDBObjDataProperty (hUserID, kPGPUserIDProperty_AttributeData, pszNotation, 256, &iNoteSize));

   return 0;
   }



// Get the fingerprint string of a key in the local keyring
//
int CPKC::GetKeyFingerprint (PSZ pszID, PSZ pszFingerprint)
   {
   PGPSize        iLength;
   PGPByte        bFingerprint [64];
   PGPKeyDBObjRef hKey      = kInvalidPGPKeyDBObjRef;

   RETURN_IF_ERROR (GetMatchingKey (pszID, &hKey, TRUE));

   // get it's binary fingerprint
   RETURN_IF_ERROR (PGPGetKeyDBObjDataProperty (hKey, kPGPKeyProperty_Fingerprint, &bFingerprint, sizeof (bFingerprint), &iLength));

   // format fingerprint as a string
   FormatFingerprint (pszFingerprint, bFingerprint, iLength);

   return 0;
   }


// internal, used by GetKeyFingerprint ()
// formats a binary fingerprint as a string
//
int CPKC::FormatFingerprint (PSZ pszFingerprint, PGPByte* pbFingerprint, PGPSize iLength)
   {
   char szTmp[8];
   unsigned i;
   *pszFingerprint = '\0';

   for (i=0; i<iLength; i++)
      {
      sprintf (szTmp, "%2.2x ", (UINT)(UCHAR)pbFingerprint[i]);
      strcat (pszFingerprint, szTmp);
      }
   if (i)
      pszFingerprint [i * 3 - 1] = '\0';
   return i * 3 - 1;
   }


// internal
// returns a keyset contain keys that match a UserID string
//
PGPError CPKC::GetMatchingKeys (PSZ pszID, PGPKeySetRef* phKeySet, BOOL bExact)
   {
   PGPFilterRef hFilter = kInvalidPGPFilterRef;

   // make sure main key DB has been opened already
   RETURN_IF_ASSERTERROR (m_hKeyDB != kInvalidPGPKeyDBRef, PKCERR_NO_KEYRING);

   // create filter
   PGPError err = PGPNewKeyDBObjDataFilter (m_hContext, 
                  kPGPUserIDProperty_Name, 
                  pszID, 
                  strlen(pszID), 
                  (bExact ? kPGPMatchCriterion_Equal : kPGPMatchCriterion_SubString), 
                  &hFilter);
   RETURN_IF_ERROR (err);

   // apply filter, key keyset from keydb 
   RETURN_IF_ERROR (PGPFilterKeyDB (m_hKeyDB, hFilter, phKeySet));

   // cleanup
   if (PGPFilterRefIsValid (hFilter)) PGPFreeFilter (hFilter);
   return 0;   
   }


PGPError CPKC::GetMatchingKey (PSZ pszID, PGPKeyDBObjRef* phKey, BOOL bExact)
   {
   PGPKeySetRef   hKeySet   = kInvalidPGPKeySetRef;
   PGPKeyIterRef  hIterator = kInvalidPGPKeyIterRef;
   PGPError       err;

   // build keyset of matching keys
   RETURN_IF_ERROR (GetMatchingKeys (pszID, &hKeySet, bExact));

   // create an iterator to get a key from the keyset
   RETURN_IF_ERROR (PGPNewKeyIterFromKeySet (hKeySet, &hIterator));

   // get the first key from the keyset
   err = PGPKeyIterNextKeyDBObj (hIterator, kPGPKeyDBObjType_Key, phKey);

   // cleanup
   if (PGPKeyIterRefIsValid (hIterator)) PGPFreeKeyIter (hIterator);
   if (PGPKeySetRefIsValid (hKeySet))    PGPFreeKeySet  (hKeySet);
   return err;
   }