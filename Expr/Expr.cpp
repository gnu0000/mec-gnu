// Expr.cpp
//
// (C) 2000 Info Tech Inc.
//
// Craig Fitzgerald
//
//
//


#include "..\stdafx.h"
#include <float.h>
#include <math.h>	
#include "G_Expr.h"
#include "Expr_Priv.h"

static CHAR *pszERRSTR[] =
							{"No Error",                         // 0
							 "Closing Parenthesis expected ')'", // 1
							 "Opening Parenthesis expected '('", // 2
							 "Unrecognized character",           // 3
							 "Empty String",                     // 4
							 "Function Overflow",                // 5
							 "Function param out of range",      // 6
							 "Illegal function value",           // 7
							 "Function result out of range",     // 8
							 "Divide by zero error",             // 9
							 "Colon Expected ':'",               // 10
							 "Not enough memory",                // 11
							 "Cannot change identifier type",    // 12
							 "Function not defined",    			 // 13
							 "Comma Expected ','",               // 14
							 "Assignment only allowed on Vars",  // 15
							 "Function parameter expected",  	 // 16
							 NULL};                                  


///////////////////////////////////////////////////////////////////////////

 void CExpr::_Init ()
	{
	m_pMem   = new CExprMem;
	m_pParse	= new CExprParse;
	m_pExec  = new CExprExec;

	m_pParse->Init (m_pMem);
	m_pExec->Init (0);
	m_iError = 0;
	m_d = 0;
	}


CExpr::CExpr ()
	{
	_Init ();
	}

CExpr::CExpr (LPCTSTR pszExpr)
	{
	_Init ();
	Eval (pszExpr);
	}

CExpr::~CExpr ()
	{
	delete m_pMem;
	delete m_pParse;
	delete m_pExec;
	}

double CExpr::Eval (LPCTSTR pszExpr)
	{
	Compile (pszExpr);
	return Exec ();
	}

int CExpr::Compile (LPCTSTR pszExpr)
	{
	m_pExec->SetErr (0);
	return (m_iError = m_pParse->Parse (pszExpr, m_pCode, m_iCodeSize));
	}

double CExpr::Exec ()
	{
	if (!m_pCode)
		return m_d = 0.0;
	if (!m_iError)
		m_d = m_pExec->Exec (m_pCode);
	return m_d;
	}

void CExpr::AddFn (LPCTSTR pszIdent, MATHFN0 pfn)
	{
	m_pMem->SetFn (pszIdent, pfn, DAT_FNREF, 0);
	}

void CExpr::AddFn (LPCTSTR pszIdent, MATHFN1 pfn)
	{
	m_pMem->SetFn (pszIdent, pfn, DAT_FNREF, 1);
	}

void CExpr::AddFn (LPCTSTR pszIdent, MATHFN2 pfn)
	{
	m_pMem->SetFn (pszIdent, pfn, DAT_FNREF, 2);
	}

void CExpr::AddFn (LPCTSTR pszIdent, MATHFN3 pfn)
	{
	m_pMem->SetFn (pszIdent, pfn, DAT_FNREF, 3);
	}

void CExpr::AddFn (LPCTSTR pszIdent, MATHFN4 pfn)
	{
	m_pMem->SetFn (pszIdent, pfn, DAT_FNREF, 4);
	}

void CExpr::AddVar (LPCTSTR pszIdent, double dVal)
	{
	m_pMem->SetVar (pszIdent, dVal);
	}

void CExpr::AddVar (LPCTSTR pszIdent, int iVal)
	{
	m_pMem->SetVar (pszIdent, (double)iVal);
	}

void CExpr::AddVarRef (LPCTSTR pszIdent, double* pdVal)
	{
	m_pMem->SetVar (pszIdent, pdVal);
	}

void CExpr::AddVarRef (LPCTSTR pszIdent, int* piVal)
	{
	m_pMem->SetVar (pszIdent, piVal);
	}

void CExpr::AddTiedVar (LPCTSTR pszIdent, MATHTIEFN pfnTie)
	{
	m_pMem->SetTiedVar (pszIdent, pfnTie);
	}


BOOL CExpr::GetVar (LPCTSTR pszIdent, double* pVal)
	{
	CExprData* pData = m_pMem->GetVar (pszIdent, FALSE);
	if (!pData)
		return FALSE;
	if (pVal)
		*pVal = pData->m_dVal;
	return TRUE;
	}

double* CExpr::GetVarRef (LPCTSTR pszIdent)
	{
	CExprData* pData = m_pMem->GetVar (pszIdent, FALSE);
	if (!pData)
		return NULL;
	return (double*)(pData->m_ptr);
	}

//void CExpr::ShareScope (CExpr* pExpr)
//	{
//	delete m_pMem;
//	m_pMem = pExpr->GetScope ();
//	m_bLocalMem = FALSE;
//	}

//BOOL CExpr::UseGlobalScope (BOOL bGlobal)
//	{
//	}


void CExpr::EmptyStringIsError (BOOL bError)
	{
	m_pParse->m_bEmptyStringIsError = bError;
	}

int CExpr::IsError (LPCTSTR &strErr, int &iIndex)
	{
	int iErr; 
	
	// Did we get a parse error?
	if (iErr = m_pParse->IsError (&strErr, &iIndex))
		return iErr;
	iIndex = 0;
	return m_pExec->IsError (&strErr);
	}

#if defined (_DEBUG)

int CExpr::Dump (LPCTSTR strFile)
	{
	CStdioFile cFile;

	if (!cFile.Open (strFile, CFile::modeCreate | CFile::modeWrite))
		return 1;

	m_pMem->DumpVars (&cFile);
	m_pMem->DumpFns (&cFile, m_pExec);

	if (m_pCode)
		{
		cFile.WriteString ("MAIN:\n");
		m_pExec->DumpCS (&cFile, m_pCode);
		}
	return 0;
	}
#endif //if defined (_DEBUG)


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

CExprParse::CExprParse ()
	{
	m_iError  				 = 0;
	m_pszError 				 = NULL;
	m_iErrIdx   			 = 0;
	m_bEmptyStringIsError = TRUE;
	}

void CExprParse::Init (CExprMem* pMemHandler)
	{
	m_pMem = pMemHandler;
	}


int CExprParse::ParseFunctionCall (PSZ psz)
	{
	CExprData* pData = m_pMem->GetFn (psz, FALSE);
	if (!pData)
		return SetErr (13);

	for (int i= pData->m_cParamCount; i; i--)
		{
		ParseExpression (0, TRUE);
		if (i > 1 && !Eat (","))
			return SetErr (14);
		}
	if (!Eat (")"))
		return SetErr (1);
	
	if (pData->m_cType == DAT_FNREF)
		m_gen.Add (OP_CALLE, (DWORD)(pData->m_ptr));
	else if (pData->m_ptr)
		m_gen.Add (OP_CALL,  (DWORD)(pData->m_ptr));
	else
		m_gen.Add ((BYTE)OP_CALLS); // call self
	m_gen.Add (pData->m_cParamCount);
	return m_iError;
	}


int CExprParse::FindStackVar (PSZ pszIdent)
	{
	int i;
	if (m_StkVarMap.Lookup (pszIdent, i))
		return m_iParamCount - i;
	else return 0;
	}

//	We parse and codegen at the same time, so i make assumptions:
//	Here we assume the variable is a RValue reference
//	If it later turns out that this is an LValue followed
//	by an assignment op, it will be replaced
int CExprParse::AddVariable (PSZ psz)
	{
	int iIndex = FindStackVar (psz);
	if (iIndex)
		return m_gen.Add ((BYTE)OP_LOADSD, (BYTE)iIndex);

	CExprData* pData = m_pMem->GetVar (psz, TRUE);

	if (pData->m_cType == DAT_VAR)
		m_gen.Add (OP_LOADD, (DWORD)&pData->m_dVal);
	else if (pData->m_cType == DAT_VARREF)
		m_gen.Add (OP_LOADD, (DWORD)(pData->m_ptr));
	else if (pData->m_cType == DAT_VARIREF)
		m_gen.Add (OP_LOADDI, (DWORD)(pData->m_ptr));
	else if (pData->m_cType == DAT_VARTIED)
		m_gen.Add (OP_CALLTL, (DWORD)(pData->m_ptr));
	return 0;
	}


int CExprParse::AddConst (double d)
	{
	m_gen.Add (OP_PUSHD, d);
	return m_iError;
	}

int CExprParse::SkipWhite (PSZ &psz)
	{
	while (*psz && strchr (" \t", *psz)) 
		psz++;
	return *psz;
	}

int CExprParse::SkipWhite ()
	{
	return SkipWhite (m_pszPtr);
//
//	while (*m_pszPtr && strchr (" \t", *m_pszPtr)) 
//		m_pszPtr++;
//	return *m_pszPtr;
	}


BOOL CExprParse::IsNumber ()
	{
	PSZ pszTmp = m_pszPtr;
	if (*pszTmp && strchr ("-+", *pszTmp))
		{
		pszTmp++;
		SkipWhite (pszTmp);
		}
	return (*pszTmp && strchr ("0123456789.", *pszTmp));
	}

#define NUMSIZE 128

double CExprParse::ReadNumber ()
	{
	double d;
	CHAR szTmp[NUMSIZE];
	PSZ  p2, pszEnd;

	p2 = szTmp;
	if (*m_pszPtr && strchr ("+-", *m_pszPtr))
		*p2++ = *m_pszPtr++;
	SkipWhite ();
	if (*m_pszPtr == '.')
		*p2++ = '0';
	strncpy (p2, m_pszPtr, NUMSIZE-3);
	szTmp[NUMSIZE-1] = '\0';

	d = strtod (szTmp, &pszEnd);
	m_pszPtr += (pszEnd - p2);	
	return d;
	}

void CExprParse::ReadIdentifier (PSZ psz, int iMaxLen)
	{
	*psz++ = *m_pszPtr++;
	for (int i=0; i+1<iMaxLen && __iscsym (*m_pszPtr); i++)
		*psz++ = *m_pszPtr++;
	*psz = '\0';
	}

int CExprParse::Eat (PSZ pszList)
	{
	if (!*m_pszPtr)
		return 0;
	SkipWhite ();
	if (!strchr (pszList, *m_pszPtr))
		return 0;
	return *m_pszPtr++;
	}

BYTE CExprParse::EatOp (int iLevel)
	{
	INT iLen;
	POP pop;

	SkipWhite ();
	for (pop = OPERANDS; pop->iOP; pop++)
		{
		iLen = strlen (pop->pszOP);
		if (strncmp (m_pszPtr, pop->pszOP, iLen))
			continue;

		/*--- found op but at wrong presidence level ---*/
		if (pop->iLevel != iLevel)
			return 0;

		m_pszPtr += iLen;
		return pop->iOP;
		}
	return 0; // no match
	}



int CExprParse::ParseAtom ()
	{
	SkipWhite ();

	/*--- unary ops ---*/
	while (Eat("+"))
		;
	if (Eat("-"))
		{
		int iRet = ParseAtom ();
		m_gen.Add ((BYTE)OP_NEG);
		return iRet;
		}

	/*--- a number ---*/
	if (IsNumber ())
		return AddConst (ReadNumber());

	/*--- a parenthesized expression ---*/
	if (Eat("("))
		{
		ParseExpression (0, FALSE);
		if (!Eat (")"))
			return SetErr (1);
		return m_iError;
		}

	/*--- an identifier ---*/
	if (__iscsymf (*m_pszPtr))
		{
		char szIdent[MAX_IDENTLEN];

		ReadIdentifier (szIdent, MAX_IDENTLEN);
		SkipWhite ();
		if (Eat("("))
			return ParseFunctionCall (szIdent);
		else
			return AddVariable (szIdent);
		}
	return SetErr (3);
	}



int CExprParse::ParseExpression (int iLevel, BOOL bStopOnComma)
	{
	if (iLevel == ATOMLEVEL)
		return ParseAtom ();

	if (ParseExpression (iLevel+1, bStopOnComma))
		return m_iError;

	BYTE cOp;
	while (TRUE)
		{
	   switch (cOp = EatOp (iLevel))
			{
			case OP_EXP :
			case OP_MOD :
			case OP_DIV :
			case OP_MUL :
			case OP_ADD :
			case OP_SUB :
			case OP_SRT :
			case OP_SLT :
			case OP_GT  :
			case OP_LT  :
			case OP_GE  :
			case OP_LE  :
			case OP_EQU :
			case OP_NEQ :
			case OP_AND :
			case OP_OR  :
			case OP_BAND:
			case OP_BOR :
				ParseExpression (iLevel+1, bStopOnComma);
				m_gen.Add (cOp);
				break;

			case OP_SET :
				{
				// cvt RValue into LValue
				BYTE cLastOp = m_gen.LastOp();
				if (!m_gen.IsLoadOp (cLastOp))
					return SetErr (15);
				DWORD wAddr = m_gen.GetLastAddress ();
				m_gen.Backup (5);
				ParseExpression (iLevel+1, bStopOnComma);
				m_gen.Add(m_gen.CvtLoadToSave (cLastOp), wAddr);
				}
				break;
					
			case OP_SADD:
			case OP_SSUB:
			case OP_SMOD:
			case OP_SDIV:
			case OP_SMUL:
				{
				BYTE cLastOp = m_gen.LastOp();
				if (! m_gen.IsLoadOp (cLastOp))
					return SetErr (15);
				DWORD wAddr = m_gen.GetLastAddress ();
				ParseExpression (iLevel+1, bStopOnComma);
				m_gen.Add ((BYTE)m_gen.CvtAssignToOp (cOp));
				m_gen.Add ((BYTE)m_gen.CvtLoadToSave(cLastOp), (DWORD)wAddr);
				}
				break;

			case OP_COND:
				{
				int iCodeLoc1 = m_gen.Add (OP_JZ, (DWORD)0);
				ParseExpression (iLevel, bStopOnComma);
				int iCodeLoc2 = m_gen.Add (OP_JMP, (DWORD)0);
				Eat (":");
				m_gen.PatchAddr (iCodeLoc1);
				ParseExpression (iLevel, bStopOnComma);
				m_gen.PatchAddr (iCodeLoc2);
				}
				break;

			case OP_COMA:
				if (bStopOnComma)
					{
					m_pszPtr--; // unget the comma
					return 0;
					}
				ParseExpression (iLevel+1, bStopOnComma);
				m_gen.Add ((BYTE)OP_POP);
				break;

			default:
				return 0;
			}
		}
	}


BOOL CExprParse::IsFunctionDefinition ()
	{
	SkipWhite ();
	if (!__iscsymf (*m_pszPtr))
		return FALSE;
	m_pszPtr++;
	while (__iscsym (*m_pszPtr))
		m_pszPtr++;
	if (!Eat("("))
		return FALSE;
	for (; *m_pszPtr && *m_pszPtr != ')'; m_pszPtr++)
		;
	if (*m_pszPtr != ')')
		return FALSE;
	m_pszPtr++;
	return Eat("=");
	}


int CExprParse::ParseParam (int iIndex)
	{
	SkipWhite ();
	if (!__iscsymf (*m_pszPtr))
		return SetErr (16);
	char szIdent[MAX_IDENTLEN];

	ReadIdentifier (szIdent, MAX_IDENTLEN);
	m_StkVarMap[szIdent] = iIndex;
	return 0;
	}


int CExprParse::ParseFunctionDefinition ()
	{
	m_iParamCount = 0;

	char szIdent [MAX_IDENTLEN];
	ReadIdentifier (szIdent, MAX_IDENTLEN);

	if (!Eat ("("))
		return SetErr (2);

	for (m_iParamCount=0; !Eat(")"); )
		{
		if (ParseParam (m_iParamCount))
			return SetErr (3);
		m_iParamCount++;
		if (Eat (")"))
			break;
		if (!Eat (","))
			return SetErr (14);
		}
	if (!Eat ("="))
		return SetErr (3);

	CExprData* pData = m_pMem->SetFn (szIdent, NULL, DAT_FNSTR, m_iParamCount);


	if (ParseExpression (0, FALSE))
		{
		m_pMem->DelFn (szIdent);
		return SetErr (3);
		}
	m_gen.Add ((BYTE)0);	// terminator opcode

	BYTE* pCode;
	int	iCodeSize;
	m_gen.Get (pCode, iCodeSize);

#if defined (_DEBUG)
	pData->m_ptr = new BYTE[iCodeSize+DEBUG_EXTRA_BYTES];
	memcpy (pData->m_ptr, pCode, iCodeSize);
	memset (((BYTE*)pData->m_ptr)+iCodeSize, 0xFF, DEBUG_EXTRA_BYTES);
	pData->m_str = m_psz;
#else	
	pData->m_ptr = new BYTE[iCodeSize];
	memcpy (pData->m_ptr, pCode, iCodeSize);
#endif

	return 0;
	}


int CExprParse::Parse (LPCTSTR pszExpr, BYTE* &pCode, int &iSize)
	{
	int iRet;

	m_gen.Init (0);
	m_StkVarMap.RemoveAll ();
	m_iError = 0;
	m_psz = m_pszPtr = (PSZ)pszExpr;
	pCode = NULL;

	if (IsFunctionDefinition ())
		{
		m_pszPtr = m_psz;
		iRet = ParseFunctionDefinition ();
		m_gen.Add ((BYTE)0);
		iSize = 0;
		return iRet;
		}
	if (!m_psz || !*m_psz)
		return (m_bEmptyStringIsError ? SetErr (4) : 0);

	m_pszPtr = m_psz;
	iRet = ParseExpression (0, FALSE);
	m_gen.Add ((BYTE)0);
	m_gen.Get (pCode, iSize);
	return iRet;
	}

int CExprParse::SetErr (int iErr)
	{
	if (m_iError)
		return m_iError;

	m_iError   = iErr;
	m_pszError = pszERRSTR [m_iError];
	m_iErrIdx  = m_pszPtr - m_psz;
	return m_iError;
	}

int CExprParse::IsError (LPCTSTR* ppszErrStr, int* piErrIdx)
	{
	if (piErrIdx)
		*piErrIdx = m_iErrIdx;
	if (ppszErrStr)
		*ppszErrStr = m_pszError;

	return m_iError;
	}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//	possible codesize reduction optimizations:
//	OP_PUSHD0 and OP_PUSHD1 opcodes
//	OP_LOADSD1and OP_SAVSD1 opcodes
//

CExprGen::CExprGen ()
	{
	m_pCS = NULL;
	}

void CExprGen::Init (int iMaxCodeSize)
	{
	if (!m_pCS)
		{
		m_iCSSize = (iMaxCodeSize ? iMaxCodeSize : DEFAULT_SIZE_CODE);
		m_pCS = new BYTE [m_iCSSize];
		}
	m_iIP = 0;

#if defined (_DEBUG)
	memset (m_pCS, 0xFF, m_iCSSize);
#endif
	}

BYTE* CExprGen::Get (BYTE* &pCode, int &iSize)
	{
	pCode = m_pCS;
	iSize = m_iIP;
	return m_pCS;
	}

int CExprGen::Add (BYTE c)
	{
	if (m_iIP + (int)sizeof(BYTE) > m_iCSSize)
		Grow();
	m_pCS [m_iIP++] = m_cLastOp = c;
	return m_iIP;
	}

int CExprGen::Add (DWORD dw)
	{
	if (m_iIP+(int)sizeof(DWORD) > m_iCSSize)
		Grow();
	*(DWORD*)(m_pCS + m_iIP) = dw;
	m_iIP += sizeof (DWORD);
	return m_iIP;
	}

int CExprGen::Add (double d)
	{
	if (m_iIP+(int)sizeof(double) > m_iCSSize)
		Grow();
	*(double*)(m_pCS + m_iIP) = d;
	m_iIP += sizeof (double);
	return m_iIP;
	}

// returned val is the addr of the operand
int CExprGen::Add (BYTE c, BYTE c2)
	{
	int iRet = Add (c);
	// Add (c2); this messes up cLastOp
	if (m_iIP + (int)sizeof(BYTE) > m_iCSSize)
		Grow();
	m_pCS [m_iIP++] = c2;
	return iRet;
	}

// returned val is the addr of the operand
int CExprGen::Add (BYTE c, DWORD dw)
	{
	int iRet = Add (c);
	Add (dw);
	return iRet;
	}

// returned val is the addr of the operand
int CExprGen::Add (BYTE c, double d)
	{
	int iRet = Add (c);
	Add (d);
	return iRet;
	}

// this patches a jump operand to jump to the current location
int CExprGen::PatchAddr (int iOffset)
	{
	return *(DWORD*)(m_pCS + iOffset) = m_iIP - iOffset - sizeof (DWORD);
	}

BYTE 	CExprGen::LastOp()
	{
	return m_cLastOp;
	}

// if the last op was a load op, this returns the load addr
// this is needed to convert a rval reference into an lval for an assignment
DWORD CExprGen::GetLastAddress ()
	{
	return *(DWORD*)(m_pCS + m_iIP - sizeof (DWORD));
	}

void 	CExprGen::Backup (int iBytes)
	{
	m_iIP -= iBytes;
	}

void  CExprGen::Grow ()
	{
	BYTE *pNew = new BYTE [m_iCSSize + CODESIZE_INCREMENT];
	memcpy (pNew, m_pCS, m_iCSSize);
	m_iCSSize += CODESIZE_INCREMENT;

	delete m_pCS;
	m_pCS = pNew;
	}


BOOL CExprGen::IsLoadOp (BYTE cOP)
	{
	return (cOP == OP_LOADD  || 
	        cOP == OP_LOADSD || 
	        cOP == OP_LOADDI ||
	        cOP == OP_CALLTL);
	}

BYTE CExprGen::CvtLoadToSave (BYTE cOP)
	{
	switch (cOP)
		{
		case OP_LOADD : return OP_SAVD;
		case OP_LOADDI: return OP_SAVDI;
		case OP_LOADSD: return OP_SAVSD;
		case OP_CALLTL: return OP_CALLTS;
		}
	return 0;
	}

BYTE CExprGen::CvtAssignToOp (BYTE cOP)
	{
	switch (cOP)
		{
		case OP_SADD: return OP_ADD;
		case OP_SSUB: return OP_SUB;
		case OP_SMOD: return OP_MOD;
		case OP_SDIV: return OP_DIV;
		case OP_SMUL: return OP_MUL;
		}
	return 0;
	}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

CExprExec::CExprExec ()
	{
	m_pSS = NULL;
	m_iError = 0;
	}

CExprExec::~CExprExec ()
	{
	if (m_pSS)
		delete m_pSS;
	}

void CExprExec::Init (int iStackSize)
	{
	if (!m_pSS)
		{
		m_iSSSize = (iStackSize ? iStackSize : DEFAULT_SIZE_STACK);
		m_pSS = new double [m_iSSSize];
		}
	m_iSP  = 0;
	m_iError = 0;

#if defined (_DEBUG)
	memset (m_pSS, 0xFF, m_iSSSize);
#endif

	}

// stack op macros
#define Pushd(d)  (m_pSS[++m_iSP]=(d))
#define Popd()	   (m_pSS[m_iSP--])
#define Peekd(i)	(m_pSS[i])
#define Poked(i,d)(m_pSS[i]=(d))
#define sv(i)     (m_pSS[m_iSP+(i)])

// code op macros & fns
#define Nextb() 	(pCS[iIP++])

double Nextd (BYTE* pCS, int &iIP)
	{
	double d = (*(double*)(pCS+iIP));
	iIP+=sizeof(double);
	return d;
	}

DWORD Nextw (BYTE* pCS, int &iIP)
	{
	DWORD wd = (*(DWORD*)(pCS+iIP));
	iIP+=sizeof(DWORD);
	return wd;
	}


double CExprExec::ExecExternal (DWORD dw, int iParams)
	{
	switch (iParams)
		{
		case 0: 
			{
			MATHFN0 pfn = (MATHFN0)(void*)dw;  
			return pfn ();
			}
		case 1: 
			{
			MATHFN1 pfn = (MATHFN1)(void*)dw;  
			return pfn (Popd ());
			}
		case 2: 
			{
			MATHFN2 pfn = (MATHFN2)(void*)dw;  
			double d2 = Popd ();
			double d1 = Popd ();
			return pfn (d1,d2);
			}
		case 3: 
			{
			MATHFN3 pfn = (MATHFN3)(void*)dw;  
			double d3 = Popd ();
			double d2 = Popd ();
			double d1 = Popd ();
			return pfn (d1,d2,d3);
			}
		case 4: 
			{
			MATHFN4 pfn = (MATHFN4)(void*)dw;  
			double d4 = Popd ();
			double d3 = Popd ();
			double d2 = Popd ();
			double d1 = Popd ();
			return pfn (d1,d2,d3,d4);
			}
		}
	return 0.0;
	}

double CExprExec::ExecTieFn (DWORD dw, BOOL bSave)
	{
	MATHTIEFN pfn = (MATHTIEFN)(void*)dw;
	return pfn ((bSave ? Popd() : 0), bSave);
	}

int CExprExec::IsError	(LPCTSTR* ppszErrStr)
	{
	if (ppszErrStr)
		*ppszErrStr = pszERRSTR [m_iError];
	return m_iError;
	}


int CExprExec::SetErr (int iErr)
	{
	return (m_iError = iErr);
	}


int CExprExec::fpErrorCheck (int iStatus)
	{
	if (iStatus & _EM_UNDERFLOW ) return (m_iError = 0); // allow this
	if (iStatus & _EM_OVERFLOW  )	return (m_iError = 8);
	if (iStatus & _EM_ZERODIVIDE)	return (m_iError = 9);
	if (iStatus & _EM_INVALID   )	return (m_iError = 7);
	return (m_iError = 0); 
	}


//	current implementation: using the real stack for calls
//	possible change: push context on vm stack and never leave this fn until done, will
//	  need to use a RET opcode for fn's and an END for main though, but then can use 
//	  helper methods like push and pop because CS,SS,IP AND SP can become member variables
//   rather than locals.
//
double CExprExec::Exec (BYTE* pCS)
	{
	double* pd;
	double d;
	int	iSF = m_iSP;
	int	iIP = 0;
	int	i, *pi;
	BYTE	cOP;
	BYTE  b;
	DWORD dw;

	if (!pCS || !*pCS)
		return 0.0;

	_clearfp (); // clear floating point status word
	m_iError = 0;

	while (cOP = Nextb ())
		{
		switch (cOP)
			{
//			case OP_PUSHD 	: Pushd	(Nextd (pCS,iIP)); 											break;
//			case OP_SAVD  	: pd=(double*)Nextw(pCS,iIP); *pd=Peekd (m_iSP);				break;
//			case OP_SAVSD 	: Poked (iSF-Nextb()+1, Peekd(m_iSP));								break;
//			case OP_LOADD 	: Pushd (*(double*)Nextw(pCS,iIP));									break;
//			case OP_LOADSD	: Pushd (Peekd(iSF-Nextb()+1));										break;
//			case OP_CALL  	: d = Exec ((BYTE*)Nextw (pCS,iIP)); m_iSP-=Nextb();Pushd (d);	break;
//			case OP_CALLS 	: d = Exec (pCS); m_iSP-=Nextb();Pushd (d);							break;
//			case OP_CALLE 	: Pushd (ExecExternal (Nextw (pCS,iIP), Nextb())); 			break;

			case OP_PUSHD 	: 
				d = Nextd (pCS,iIP);
				Pushd	(d); 											
				break;
			case OP_SAVD  	: 
				pd=(double*)Nextw(pCS,iIP); 
				*pd=Peekd (m_iSP);				
				break;
			case OP_SAVDI 	: 
				pi=(int*)Nextw(pCS,iIP); 
				*pi=(int)Peekd (m_iSP);				
				break;
			case OP_SAVSD 	: 
				i = iSF-Nextb()+1;
				d  = Peekd(m_iSP);
				Poked (i, d);								
				break;
			case OP_LOADD 	: 
				d = *(double*)Nextw(pCS,iIP);
				Pushd (d);									
				break;
			case OP_LOADDI	: 
				d = (double)*(int*)Nextw(pCS,iIP);
				Pushd (d);									
				break;
			case OP_LOADSD	: 
				i = iSF-Nextb()+1;
				d = Peekd(i);
				Pushd (d);										
				break;
			case OP_CALL  	: 
				d = Exec ((BYTE*)Nextw (pCS,iIP)); 
				m_iSP-=Nextb();
				Pushd (d);	
				break;
			case OP_CALLS 	: 
				d = Exec (pCS); 
				m_iSP-=Nextb();
				Pushd (d);							
				break;
			case OP_CALLE 	: 
				dw = Nextw (pCS,iIP);
				b = Nextb();
				d = ExecExternal (dw, b);
				Pushd (d); 			
				break;
			case OP_CALLTL:
				d = ExecTieFn (Nextw (pCS,iIP), FALSE);
				Pushd (d); 			
				break;
			case OP_CALLTS:
				d = ExecTieFn (Nextw (pCS,iIP), TRUE);
				Pushd (d); 			
				break;

			case OP_JMP   	: iIP += Nextw(pCS,iIP); 												break;
			case OP_JZ    	: i=Nextw(pCS,iIP); if (sv(0) == 0.0) iIP += i;	m_iSP--;		break;
			case OP_JNZ   	: i=Nextw(pCS,iIP); if (sv(0) != 0.0) iIP += i;	m_iSP--;		break;
			case OP_EXP 	: sv(-1) = pow (sv(-1), sv(0)); m_iSP--;							break;
			case OP_MOD 	: sv(-1) = fmod (sv(-1), sv(0)); m_iSP--;							break;
			case OP_DIV 	: sv(-1) /= sv(0); m_iSP--;											break;
			case OP_MUL 	: sv(-1) *= sv(0); m_iSP--;											break;
			case OP_ADD 	: sv(-1) += sv(0); m_iSP--;											break;
			case OP_SUB 	: sv(-1) -= sv(0); m_iSP--;											break;
			case OP_SRT 	: sv(-1) /= pow((double)10, sv(0)); m_iSP--;								break;
			case OP_SLT 	: sv(-1) *= pow((double)10, sv(0)); m_iSP--;								break;
			case OP_GT  	: sv(-1) = (sv(-1) >  sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_LT  	: sv(-1) = (sv(-1) <  sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_GE  	: sv(-1) = (sv(-1) >= sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_LE  	: sv(-1) = (sv(-1) <= sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_EQU 	: sv(-1) = (sv(-1) == sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_NEQ 	: sv(-1) = (sv(-1) != sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_AND 	: sv(-1) = (sv(-1) && sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_OR  	: sv(-1) = (sv(-1) || sv(0) ? 1.0 : 0.0); m_iSP--; 			break;
			case OP_BAND	: sv(-1) = (double)((ULONG)sv(-1) & (ULONG)sv(0)); m_iSP--; break;
			case OP_BOR 	: sv(-1) = (double)((ULONG)sv(-1) | (ULONG)sv(0)); m_iSP--; break; 
			case OP_NEG  	: sv(0)  = - sv(0);														break;
			}
		}
	fpErrorCheck (_statusfp ());
	return Popd ();
	}


#if defined (_DEBUG)

int CExprExec::DumpCS (CStdioFile* pcFile, BYTE *pCS)
	{
	int	iSF = m_iSP;
	int	iIP = 0;
	BYTE	cOP;
	double d;
	CString strAddr, strOp, strA;
	DWORD w;
	BYTE  c;

	while (TRUE)
		{
		strAddr.Format ("%4.4x", iIP);
		cOP = Nextb ();
		strA.Empty ();
		switch (cOP)
			{
			case OP_PUSHD 	: strOp="OP_PUSHD  "; d=Nextd (pCS,iIP);				strA.Format("%f", d);				 break;
			case OP_SAVD  	: strOp="OP_SAVD   "; w=Nextw(pCS,iIP);				strA.Format("%x", w);				 break;
			case OP_SAVDI 	: strOp="OP_SAVDI  "; w=Nextw(pCS,iIP);				strA.Format("%x", w);				 break;
			case OP_SAVSD 	: strOp="OP_SAVSD  "; c=Nextb();							strA.Format("%x", (int)c);			 break;
			case OP_LOADD 	: strOp="OP_LOADD  "; w=Nextw(pCS,iIP);				strA.Format("%x", w);				 break;
			case OP_LOADDI	: strOp="OP_LOADDI "; w=Nextw(pCS,iIP);				strA.Format("%x", w);				 break;
			case OP_LOADSD	: strOp="OP_LOADSD "; c=Nextb();							strA.Format("%x", (int)c);			 break;
			case OP_CALL  	: strOp="OP_CALL   "; w=Nextw(pCS,iIP); c=Nextb();	strA.Format("%x %x", w, (int)c);	 break;
			case OP_CALLS 	: strOp="OP_CALLS  "; c=Nextb();							strA.Format("%x", (int)c);	 break;
			case OP_CALLE 	: strOp="OP_CALLE  "; w=Nextw(pCS,iIP); c=Nextb();	strA.Format("%x %x", w, (int)c);	 break;
			case OP_JMP   	: strOp="OP_JMP    "; w=Nextw(pCS,iIP);  	strA.Format("%x (%x)", w+iIP, w); break;
			case OP_JZ    	: strOp="OP_JZ     "; w=Nextw(pCS,iIP);  	strA.Format("%x (%x)", w+iIP, w); break;
			case OP_JNZ   	: strOp="OP_JNZ    "; w=Nextw(pCS,iIP);  	strA.Format("%x (%x)", w+iIP, w); break;
			case OP_EXP 	: strOp="OP_EXP 	 "; break;
			case OP_MOD 	: strOp="OP_MOD 	 "; break;
			case OP_DIV 	: strOp="OP_DIV 	 "; break;
			case OP_MUL 	: strOp="OP_MUL 	 "; break;
			case OP_ADD 	: strOp="OP_ADD 	 "; break;
			case OP_SUB 	: strOp="OP_SUB 	 "; break;
			case OP_SRT 	: strOp="OP_SRT 	 "; break;
			case OP_SLT 	: strOp="OP_SLT 	 "; break;
			case OP_GT  	: strOp="OP_GT  	 "; break;
			case OP_LT  	: strOp="OP_LT  	 "; break;
			case OP_GE  	: strOp="OP_GE  	 "; break;
			case OP_LE  	: strOp="OP_LE  	 "; break;
			case OP_EQU 	: strOp="OP_EQU 	 "; break;
			case OP_NEQ 	: strOp="OP_NEQ 	 "; break;
			case OP_AND 	: strOp="OP_AND 	 "; break;
			case OP_OR  	: strOp="OP_OR  	 "; break;
			case OP_BAND	: strOp="OP_BAND	 "; break;
			case OP_BOR 	: strOp="OP_BOR 	 "; break;
			case OP_NEG 	: strOp="OP_NEG 	 "; break;
			case 0         : strOp="END       "; break;
			}
		pcFile->WriteString ((CString)"   " + strAddr + "  " + strOp + strA + "\n");
		if (!cOP)
			break;
		}
	pcFile->WriteString ("======================================\n\n\n");
	return 0;
	}

#endif // defined (_DEBUG)

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

// not yet thread safe!!
//
BOOL CExprMem::m_bInitialized = FALSE;
CExprDataMap CExprMem::m_GlobalFnMap;
CExprDataMap CExprMem::m_GlobalVarMap;


void CExprMem::InitGlobals ()
	{
	UseGlobalScope (TRUE);

	SetVar ("pi", 3.1415926535);
	SetVar ("e" , 2.302585    );

//
//  added (MATHFN1) to following SetFn calls to get a clean compile under VS 2003.
//	
	SetFn ("sin",   (void*) (MATHFN1) sin  , DAT_FNREF, 1);
	SetFn ("cos",   (void*) (MATHFN1) cos  , DAT_FNREF, 1);
	SetFn ("tan",   (void*) (MATHFN1) tan  , DAT_FNREF, 1);
	SetFn ("asin",  (void*) (MATHFN1) asin , DAT_FNREF, 1);
	SetFn ("acos",  (void*) (MATHFN1) acos , DAT_FNREF, 1);
	SetFn ("atan",  (void*) (MATHFN1) atan , DAT_FNREF, 1);
	SetFn ("abs",   (void*) (MATHFN1) fabs , DAT_FNREF, 1);
	SetFn ("ceil",  (void*) (MATHFN1) ceil , DAT_FNREF, 1);
	SetFn ("cosh",  (void*) (MATHFN1) cosh , DAT_FNREF, 1);
	SetFn ("sinh",  (void*) (MATHFN1) sinh , DAT_FNREF, 1);
	SetFn ("exp",   (void*) (MATHFN1) exp  , DAT_FNREF, 1);
	SetFn ("floor", (void*) (MATHFN1) floor, DAT_FNREF, 1);
	SetFn ("log10", (void*) (MATHFN1) log10, DAT_FNREF, 1);
	SetFn ("log",   (void*) (MATHFN1) log  , DAT_FNREF, 1);
	SetFn ("sqrt",  (void*) (MATHFN1) sqrt , DAT_FNREF, 1);

	m_bInitialized = TRUE;

	UseGlobalScope (FALSE);
	}


CExprMem::CExprMem ()
	{
	if (!m_bInitialized)
		InitGlobals ();
	}

CExprMem::~CExprMem ()
	{
	DeleteAll ();
	}


BOOL CExprMem::UseGlobalScope (BOOL bUse)
	{
	BOOL bRet = m_bUseGlobalScope;
	m_bUseGlobalScope = bUse;
	return bRet;
	}


CExprData* CExprMem::GetVar (LPCTSTR pszIdent, BOOL bCreateIfNeeded)
	{
	CExprData* pData;
	if (m_LocalVarMap.Lookup (pszIdent, pData))
		return pData;
	if (m_GlobalVarMap.Lookup (pszIdent, pData))
		return pData;
	if (!bCreateIfNeeded)
		return NULL;

	if (m_bUseGlobalScope)
		m_GlobalVarMap[pszIdent] = pData = new CExprData;
	else
		m_LocalVarMap[pszIdent] = pData = new CExprData;
	pData->m_cType = DAT_VAR; // an assumption
	pData->m_dVal  = 0.0;
	return pData;
	}

CExprData* CExprMem::SetVar (LPCTSTR pszIdent, double d)
	{
	CExprData* pData = GetVar (pszIdent, TRUE);
	pData->m_cType = DAT_VAR;
	pData->m_dVal  = d;
	return pData;
	}

CExprData* CExprMem::SetVar (LPCTSTR pszIdent, double* pd)
	{
	CExprData* pData = GetVar (pszIdent, TRUE);
	pData->m_cType = DAT_VARREF;
	pData->m_ptr = pd;
	return pData;
	}

CExprData* CExprMem::SetVar (LPCTSTR pszIdent, int* pi)
	{
	CExprData* pData = GetVar (pszIdent, TRUE);
	pData->m_cType = DAT_VARIREF;
	pData->m_ptr = pi;
	return pData;
	}


CExprData* CExprMem::SetTiedVar (LPCTSTR pszIdent, MATHTIEFN pfnTie)
	{
	CExprData* pData = GetVar (pszIdent, TRUE);
	pData->m_cType = DAT_VARTIED;
	pData->m_ptr  = (void*)pfnTie;
	return pData;
	}

void CExprMem::DelVar (LPCTSTR pszIdent)
	{
	if (m_LocalVarMap.RemoveKey (pszIdent))
		return;
	m_GlobalVarMap.RemoveKey (pszIdent);
	}


CExprData* CExprMem::GetFn (LPCTSTR pszIdent, BOOL bCreateIfNeeded)
	{
	CExprData* pData;
	if (m_LocalFnMap.Lookup (pszIdent, pData))
		return pData;
	if (m_GlobalFnMap.Lookup (pszIdent, pData))
		return pData;
	if (!bCreateIfNeeded)
		return NULL;

	if (m_bUseGlobalScope)
		m_GlobalFnMap[pszIdent] = pData = new CExprData;
	else
		m_LocalFnMap[pszIdent] = pData = new CExprData;
	pData->m_cType = DAT_FNREF; // an assumption
	pData->m_ptr   = NULL;		 // not yet valid
	return pData;
	}

CExprData* CExprMem::SetFn (LPCTSTR pszIdent, void* pAddr, BYTE cType, int iParamCount)
	{
	CExprData* pData = GetFn (pszIdent, TRUE);
	if (pData->m_cType == DAT_FNSTR)
		delete pData->m_ptr; // cleanup if it already exists and allocs mem

	pData->m_cType 		= cType;
	pData->m_cParamCount = iParamCount;
	pData->m_ptr   		= pAddr;
	return pData;
	}

void CExprMem::DelFn (LPCTSTR pszIdent)
	{
	if (m_LocalFnMap.RemoveKey (pszIdent))
		return;
	m_GlobalFnMap.RemoveKey (pszIdent);
	}

void CExprMem::DeleteAll (BOOL bGlobals)
	{
	CExprData   *pData;
	CString strKey;

	if (bGlobals)
		{
		POSITION pos = m_GlobalFnMap.GetStartPosition ();
		while (pos)
			{
			m_GlobalFnMap.GetNextAssoc (pos, strKey, pData);
			if (pData->m_cType == DAT_FNSTR)
				delete pData->m_ptr;
			}
		m_GlobalFnMap.RemoveAll ();
		m_GlobalVarMap.RemoveAll ();
		}
	else
		{
		POSITION pos = m_LocalFnMap.GetStartPosition ();
		while (pos)
			{
			m_LocalFnMap.GetNextAssoc (pos, strKey, pData);
			if (pData->m_cType == DAT_FNSTR)
				delete pData->m_ptr;
			}
		m_LocalFnMap.RemoveAll ();
		m_LocalVarMap.RemoveAll ();
		}
	}


#if defined (_DEBUG)

void CExprMem::_DumpVars (CStdioFile* pcFile, CExprDataMap& map)
	{
	CExprData*  pData;
	CString strKey, strTmp;

	pcFile->WriteString ("\n Vars:\n===========\n");
	POSITION pos = map.GetStartPosition ();
	while (pos)
		{
		map.GetNextAssoc (pos, strKey, pData);
		if (pData->m_cType == DAT_VAR)
			strTmp.Format ("Name: %s   Type:DAT_VAR     Val: %f\n", strKey, pData->m_dVal);
		else if (pData->m_cType == DAT_VARREF)
			strTmp.Format ("Name: %s   Type:DAT_VARREF  Val: %x\n\n", strKey, pData->m_ptr);
		else if (pData->m_cType == DAT_VARIREF)
			strTmp.Format ("Name: %s   Type:DAT_VARIREF Val: %x\n\n", strKey, pData->m_ptr);
		pcFile->WriteString (strTmp);
		}
	pcFile->WriteString ("\n\n");
	}

void CExprMem::DumpVars (CStdioFile* pcFile)
	{
	_DumpVars (pcFile, m_LocalVarMap);
	_DumpVars (pcFile, m_GlobalVarMap);
	}


void CExprMem::_DumpFns (CStdioFile* pcFile, CExprExec* pExec, CExprDataMap& map)
	{
	CExprData*  pData;
	CString strKey, strTmp;

	pcFile->WriteString ("\n Ref Fns:\n===============\n");
	POSITION pos = map.GetStartPosition ();
	while (pos)
		{
		map.GetNextAssoc (pos, strKey, pData);
		if (pData->m_cType == DAT_FNREF)
			{
			strTmp.Format ("Name:%s   Type:DAT_FNREF  Params:%d  Val:%d\n", strKey, (int)pData->m_cParamCount, pData->m_ptr);
			pcFile->WriteString (strTmp);
			}
		}

	pcFile->WriteString ("\n\n Fns:\n===========\n");
	pos = map.GetStartPosition ();
	while (pos)
		{
		map.GetNextAssoc (pos, strKey, pData);
		if (pData->m_cType == DAT_FNSTR)
			{
			strTmp.Format ("Name:%s   Type:DAT_FNSTR  Params:%d   Val: %d\n", strKey, (int)pData->m_cParamCount, pData->m_ptr);
			pcFile->WriteString (strTmp);
			pcFile->WriteString (CString("Str: [") + pData->m_str + "]\n");
			pExec->DumpCS (pcFile, (BYTE*)(pData->m_ptr));
			pcFile->WriteString ("------------------------------------------\n\n\n");
			}
		}
	}

void CExprMem::DumpFns (CStdioFile* pcFile, CExprExec* pExec)
	{
	_DumpFns (pcFile, pExec, m_LocalFnMap);
	_DumpFns (pcFile, pExec, m_GlobalFnMap);
	}


#endif //if defined (_DEBUG)




///* Handle several math errors caused by passing a negative argument
// * to log or log10 (_DOMAIN errors). When this happens, _matherr
// * returns the natural or base-10 logarithm of the absolute value
// * of the argument and suppresses the usual error message.
// */
//int _matherr( struct _exception *except )
//	{
//   if( except->type == _DOMAIN )
//    	{
//      if( strcmp( except->name, "log" ) == 0 )
//        	{
//         except->retval = log( -(except->arg1) );
//         printf( "Special: using absolute value: %s: _DOMAIN "
//                 "error\n", except->name );
//         return 1;
//        	}
//      else if( strcmp( except->name, "log10" ) == 0 )
//        	{
//         except->retval = log10( -(except->arg1) );
//         printf( "Special: using absolute value: %s: _DOMAIN "
//                 "error\n", except->name );
//         return 1;
//        	}
//    	}
//   printf ("Normal: ");
//   return 0;  /* Else use the default actions */
//	}


///////////////////////////////////////////////////////////////////////////
