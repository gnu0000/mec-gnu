// ExprPriv.h : Private header for CExpr
//
//	CExpr is not yet thread safe
//
//

#if !defined(EXPR_PRIV_H__INCLUDED_)
#define EXPR_PRIV_H__INCLUDED_
#pragma once


///////////////////////////////////////////////////////////////////////////
								//
#define DAT_VAR	  1	// A variable
#define DAT_VARREF  2	//	A variable reference
#define DAT_VARIREF 3	//	A variable reference (integer varient)
#define DAT_FNREF	  4	//	An External Function (ptr to C fn)
#define DAT_FNSTR	  5	//	An Internal Function	(eval string)
#define DAT_VARTIED 6	//	A Tied variable
								//
///////////////////////////////////////////////////////////////////////////

#define MAX_IDENTLEN      128

#define OP_PUSHD 	1
#define OP_LOADD 	2
#define OP_LOADDI	3
#define OP_LOADSD	4
#define OP_SAVD  	5
#define OP_SAVDI 	6
#define OP_SAVSD 	7
#define OP_CALL  	8
#define OP_CALLS 	9
#define OP_CALLE 	10
#define OP_JMP		11
#define OP_JZ    	12
#define OP_JNZ   	13
#define OP_EXP   	14
#define OP_MOD   	15
#define OP_DIV   	16
#define OP_MUL   	17
#define OP_ADD   	18
#define OP_SUB   	19
#define OP_SRT   	20
#define OP_SLT   	21
#define OP_NEG		22
#define OP_GT    	23
#define OP_LT    	24
#define OP_GE    	25
#define OP_LE    	26
#define OP_EQU   	27
#define OP_NEQ   	28
#define OP_BAND  	29
#define OP_BOR   	30
#define OP_AND   	31
#define OP_OR    	32
#define OP_COND  	33
#define OP_SET   	34
#define OP_SADD  	35
#define OP_SSUB  	36
#define OP_SMOD  	37
#define OP_SDIV  	38
#define OP_SMUL  	39
#define OP_COMA   40
#define OP_POP		41

#define OP_CALLTL	42		  // Tied Var Load
#define OP_CALLTS	43		  // Tied Var Save

///////////////////////////////////////////////////////////////////////////

typedef struct
   {
   int iLevel;
   int iOP;
   PSZ pszOP;
   } OP;
typedef OP *POP;


/*
 * Ordered so that they will match correctly when
 * searched in a linear fashion.
 * (ie '*=' will match '*=' before '*')
 */
OP OPERANDS[] =
		  {
			{9,  OP_SRT,  ">>" },
			{9,  OP_SLT,  "<<" },
			{8,  OP_GE,   ">=" }, 
			{8,  OP_LE,   "<=" }, 
			{7,  OP_EQU,  "==" }, 
			{7,  OP_NEQ,  "!=" }, 
			{4,  OP_AND,  "&&" },
			{3,  OP_OR,   "||" },
			{1,  OP_SADD, "+=" }, 
			{1,  OP_SSUB, "-=" }, 
			{1,  OP_SMOD, "%=" }, 
			{1,  OP_SDIV, "/=" }, 
			{1,  OP_SMUL, "*=" },
			{12, OP_EXP,  "^"  }, 
			{11, OP_MOD,  "%"  }, 
			{11, OP_DIV,  "/"  }, 
			{11, OP_MUL,  "*"  }, 
			{10, OP_ADD,  "+"  }, 
			{10, OP_SUB,  "-"  }, 
			{8,  OP_GT,   ">"  }, 
			{8,  OP_LT,   "<"  }, 
			{6,  OP_BAND, "&"  },
			{5,  OP_BOR,  "|"  },
			{1,  OP_SET,  "="  }, 
			{2,  OP_COND, "?"  },
			{0,  OP_COMA, ","  },  // This is an operator, so make sure to strip
			{0,  0,       NULL }}; // Terminator

#define ATOMLEVEL 13

///////////////////////////////////////////////////////////////////////////

class CExprData
	{
public:
	BYTE  m_cType;
	BYTE  m_cParamCount;
#if defined (_DEBUG)
	CString m_str;
#endif
	union
		{
		double m_dVal;
		void*  m_ptr;
		};
	};

typedef CMap<CString,LPCTSTR,CExprData*,CExprData*&> CExprDataMap;

///////////////////////////////////////////////////////////////////////////

class CExprMem
	{
public:
	CExprMem ();
	~CExprMem ();

	CExprData* GetVar (LPCTSTR pszIdent, BOOL bCreateIfNeeded);
	CExprData* SetVar (LPCTSTR pszIdent, double d);
	CExprData* SetVar (LPCTSTR pszIdent, double* pd);
	CExprData* SetVar (LPCTSTR pszIdent, int* pi);
	CExprData* SetTiedVar (LPCTSTR pszIdent, MATHTIEFN pfnTie);

	void DelVar (LPCTSTR pszIdent);

	CExprData* GetFn (LPCTSTR pszIdent, BOOL bCreateIfNeeded);
	CExprData* SetFn	(LPCTSTR pszIdent, void* pAddr, BYTE cType, int iParamCount);
	void DelFn	(LPCTSTR pszIdent);

	BOOL UseGlobalScope (BOOL bUse);
	
	void DeleteAll (BOOL bGlobals = FALSE);

#if defined (_DEBUG)
	void DumpVars (CStdioFile* pcFile);
	void _DumpVars (CStdioFile* pcFile, CExprDataMap& map);

	void DumpFns (CStdioFile* pcFile, CExprExec* pExec);
	void _DumpFns (CStdioFile* pcFile, CExprExec* pExec, CExprDataMap& map);
#endif //if defined (_DEBUG)


protected:
	BOOL m_bUseGlobalScope;
	CExprDataMap m_LocalFnMap;
	CExprDataMap m_LocalVarMap;

	static BOOL m_bInitialized;
	static CExprDataMap m_GlobalFnMap;
	static CExprDataMap m_GlobalVarMap;

	void InitGlobals ();
	};

///////////////////////////////////////////////////////////////////////////

//	this guy allocs the Stack seg
//
//	
//
class CExprExec
	{
public:
	CExprExec ();
	~CExprExec ();

	void   Init (int iStackSize);
	double Exec (BYTE* pCode);
	int	 IsError (LPCTSTR* ppszErrStr);
	int 	 SetErr (int iErr);

#if defined (_DEBUG)
	int  DumpCS (CStdioFile* pcFile, BYTE *pCS);
	void Dump (BYTE* pCode);
#endif

protected:
	BYTE* 	m_pCode;
	double*  m_pSS;
	int		m_iSSSize;
	int		m_iSP;
	int    	m_iError;

	double ExecExternal (DWORD dw, int iParams);
	double ExecTieFn (DWORD dw, BOOL bSave);
	int    fpErrorCheck (int iStatus);

	};

///////////////////////////////////////////////////////////////////////////

class CExprGen
	{
public:
	CExprGen ();
	void Init (int iMaxCodeSize);

	BYTE* Get (BYTE* &pCode, int &iSize);

	int 	Add (BYTE c);
	int 	Add (DWORD dw);
	int 	Add (double d);
	int	Add (BYTE c, BYTE c2);
	int 	Add (BYTE c, DWORD dw);
	int 	Add (BYTE c, double d);
	int 	PatchAddr (int iOffset);
	BYTE 	LastOp();
	DWORD GetLastAddress ();
	void 	Backup (int iBytes);


	BOOL  IsLoadOp (BYTE cOP);
	BYTE  CvtLoadToSave (BYTE cOP);
	BYTE  CvtAssignToOp (BYTE cOp);


protected:
	BYTE* m_pCS;
	int	m_iCSSize;
	int	m_iIP;
	BYTE  m_cLastOp;

	void Grow ();
	};
	
///////////////////////////////////////////////////////////////////////////

typedef CMap<CString,LPCTSTR,int,int> CStackVarMap;

class CExprParse
	{
public:
	CExprParse ();
	void 	Init (CExprMem* pMemHandler);
	int 	Parse (LPCTSTR pszExpr, BYTE* &pCode, int &iSize);
	int	IsError (LPCTSTR* ppszErrStr, int* piErrIdx);

	BOOL  m_bEmptyStringIsError;

protected:
	CExprGen     m_gen;
	CExprMem*    m_pMem;
	CStackVarMap m_StkVarMap;

	int		m_iError;
	LPCTSTR	m_pszError;
	int		m_iErrIdx;
	int		m_iParamCount;
	PSZ		m_psz;
	PSZ		m_pszPtr;


	int 	 ParseFunctionDefinition ();
	int 	 ParseParam (int iIndex);
	BOOL 	 IsFunctionDefinition ();
	int 	 ParseExpression (int iLevel, BOOL bStopOnComma);
	int 	 ParseAtom ();
	BYTE 	 EatOp (int iLevel);
	int 	 Eat (PSZ pszList);
	void 	 ReadIdentifier (PSZ psz, int iMaxLen);
	double ReadNumber ();
	BOOL 	 IsNumber ();
	int 	 SkipWhite (PSZ &psz);
	int 	 SkipWhite ();
	int 	 AddConst (double d);
	int 	 AddVariable (PSZ psz);
	int 	 ParseFunctionCall (PSZ psz);
	int	 FindStackVar (PSZ pszIdent);

	int 	 SetErr (int iErr);
	};

///////////////////////////////////////////////////////////////////////////


#endif // !defined (EXPR_PRIV_H__INCLUDED_)



