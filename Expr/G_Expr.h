// Expr.h
//
// (C) 2000 Info Tech Inc.
//
// Craig Fitzgerald
//
//
//	class CExpr
//
//	This class is used to evaluate math expressions. You can use it like this:
//
//		CExpr expr;
//		double d1 = expr.Eval ("100 * 6.17 * (14.447 - 3^2) + 8 % 2");
//		double d2 = expr.Eval ("x * y / sin(pi / 12)");
//
//	or like this:
//
//		CExpr expr;
//		expr.Compile ("x*100 + y*10 + z");
//		double d3 = expr.Exec ();
//
//	The 2nd method is much faster if the same expression is being executed 
//	multiple times - the parse/codegen only happens once.
//
//	CExpr supports internal and external variables and functions.  for example:
//
//		// internal vars and functions samples:
//		CExpr expr;
//		expr.Eval ("x = 100 / pi");
//		expr.Eval ("y = x^3");
//		expr.Eval ("foo(x) = x*x + x*(x/10) + x");
//		expr.Eval ("bar(x,y,z) = (x*y + y*z + x*z)^(1/3)");
//		double d4 = expr.Eval ("foo (30) + 10.5");
//		double d5 = expr.Eval ("bar (x, y, foo (y))");
//
//		// external vars samples:
//		CExpr expr;
//    double dClyde = 10.0;
//		expr.AddVar ("fred", 20.5);
//		expr.AddVarRef ("clyde", &dClyde);
//		expr.Eval ("clyde = clyde + fred");
//		assert (dClyde == 30.5)
//
//		// external fns sample:
//		expr.AddFn ("eigenval2" (MATHFN2)MyEigenValFn);
//		double d6 = expr.Eval ("eigenval2 (14, 3.33378)");
//
//
//	 The following are initially defined:
//
//	 	Vars: pi and e
//
//  	functions: 	abs()    acos()   asin()   atan()   ceil()
//    	   		cos()    cosh()   exp()    floor()  log()
//       			log10()  sin()    sinh()   sqrt()   tan()
//
//	 operator precidence (low to high):
//
//		  				+= /= =  %= *= -=  -- assignment
//		  				?                  -- ternary operator
//		  				||                 -- logical or
//		  				&&                 -- logical and
//		  				|                  -- bitwose or
//		  				&                  -- bitwise and
//		  				== !=              -- boolean comparison
//		  				>= > <= <          -- boolean comparison
//		  				<< >>              -- base 10 shift
//		  				+  -               -- add,subtract
//		  				/  %  *            -- div,mod,mult
//		  				^                  -- power
//		  				()                 -- parenthesis
//
//

#if !defined(EXPR_H__INCLUDED_)
#define EXPR_H__INCLUDED_
#pragma once


#define DEFAULT_SIZE_STACK 128
#define DEFAULT_SIZE_CODE	256
#define CODESIZE_INCREMENT 64
#define DEBUG_EXTRA_BYTES  32

typedef double (*MATHFN0) ();
typedef double (*MATHFN1) (double d1);
typedef double (*MATHFN2) (double d1, double d2);
typedef double (*MATHFN3) (double d1, double d2, double d3);
typedef double (*MATHFN4) (double d1, double d2, double d3, double d4);
typedef double (*MATHTIEFN) (double d1, BOOL bSet);


///////////////////////////////////////////////////////////////////////////

class CExprData;
class CExprMem;
class CExprExec;
class CExprGen;
class CExprParse;


class CExpr
	{
public:
	double m_d;

	CExpr ();
	CExpr	(LPCTSTR pszExpr);
	~CExpr ();
	
	double Eval (LPCTSTR pszExpr);

	int Compile (LPCTSTR pszExpr);
	double Exec ();

	void AddFn (LPCTSTR pszIdent, MATHFN0 pfn);
	void AddFn (LPCTSTR pszIdent, MATHFN1 pfn);
	void AddFn (LPCTSTR pszIdent, MATHFN2 pfn);
	void AddFn (LPCTSTR pszIdent, MATHFN3 pfn);
	void AddFn (LPCTSTR pszIdent, MATHFN4 pfn);
	void AddVar (LPCTSTR pszIdent, double dVal);
	void AddVar (LPCTSTR pszIdent, int iVal);
	void AddVarRef (LPCTSTR pszIdent, double* pdVal);
	void AddVarRef (LPCTSTR pszIdent, int* piVal);
	void AddTiedVar (LPCTSTR pszIdent, MATHTIEFN pfnTie);

	BOOL GetVar (LPCTSTR pszIdent, double* pVal);
	double* GetVarRef (LPCTSTR pszIdent);

	void ShareScope (CExpr* pExpr);
	BOOL UseGlobalScope (BOOL bGlobal);
	void EmptyStringIsError (BOOL bError);

	int IsError (LPCTSTR &strErr, int &iIndex);

#if defined (_DEBUG)
	int Dump (LPCTSTR strFile);
#endif //if defined (_DEBUG)

protected:
	CExprMem*   m_pMem;
	CExprParse* m_pParse;
	CExprExec*  m_pExec;
	int	m_iError;
	BYTE* m_pCode;
	int m_iCodeSize;

	void _Init ();
	};

///////////////////////////////////////////////////////////////////////////

#endif // !defined (EXPR_H__INCLUDED_)
