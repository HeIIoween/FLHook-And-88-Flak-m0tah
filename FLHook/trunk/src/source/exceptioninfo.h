/* Taken from Dr. Dobb's "Visual C++ Exception-Handling Instrumentation" article.
 * http://www.ddj.com/showArticle.jhtml?documentID=win0212a&pgno=6
 *
 * Small modifications made to also allow access to the current context.
 */

#if defined(_MT) && !defined(_DLL)

extern "C"
{
    // merged from VC 6 and .NET internal headers in CRT source code

    struct _tiddata 
    {
        unsigned long   _tid;       /* thread ID */


        unsigned long   _thandle;   /* thread handle */

        int     _terrno;            /* errno value */
        unsigned long   _tdoserrno; /* _doserrno value */
        unsigned int    _fpds;      /* Floating Point data segment */
        unsigned long   _holdrand;  /* rand() seed value */
        char *      _token;         /* ptr to strtok() token */
        wchar_t *   _wtoken;        /* ptr to wcstok() token */
        unsigned char * _mtoken;    /* ptr to _mbstok() token */

        /* following pointers get malloc'd at runtime */
        char *      _errmsg;        /* ptr to strerror()/_strerror()
                                       buff */
#if _MSC_VER >= 1300
        wchar_t *   _werrmsg;       /* ptr to _wcserror()/__wcserror()
                                       buff */
#endif
        char *      _namebuf0;      /* ptr to tmpnam() buffer */
        wchar_t *   _wnamebuf0;     /* ptr to _wtmpnam() buffer */
        char *      _namebuf1;      /* ptr to tmpfile() buffer */
        wchar_t *   _wnamebuf1;     /* ptr to _wtmpfile() buffer */
        char *      _asctimebuf;    /* ptr to asctime() buffer */
        wchar_t *   _wasctimebuf;   /* ptr to _wasctime() buffer */
        void *      _gmtimebuf;     /* ptr to gmtime() structure */
        char *      _cvtbuf;        /* ptr to ecvt()/fcvt buffer */

        /* following fields are needed by _beginthread code */
        void *      _initaddr;      /* initial user thread address */
        void *      _initarg;       /* initial user thread argument */

        /* following three fields are needed to support 
         * signal handling and
         * runtime errors */
        void *      _pxcptacttab;   /* ptr to exception-action table */
        void *      _tpxcptinfoptrs; /* ptr to exception info pointers*/
        int         _tfpecode;      /* float point exception code */
#if _MSC_VER >= 1300
        /* pointer to the copy of the multibyte character 
         * information used by the thread */
        /*pthreadmbcinfo*/ void *  ptmbcinfo;

        /* pointer to the copy of the locale informaton 
         * used by the thead */
        /*pthreadlocinfo*/ void *  ptlocinfo;
#endif


        /* following field is needed by NLG routines */
        unsigned long   _NLG_dwCode;

        /*
         * Per-Thread data needed by C++ Exception Handling
         */
		void *		_something;		/* otherwise fields appear to be off by one */
        void *      _terminate;     /* terminate() routine */
        void *      _unexpected;    /* unexpected() routine */
        void *      _translator;    /* S.E. translator */
        void *      _curexception;  /* current exception */
        void *      _curcontext;    /* current exception context */
#if _MSC_VER >= 1300
        int         _ProcessingThrow; /* for uncaught_exception */
#endif
#if defined (_M_IA64)
        void *      _pExitContext;
        void *      _pUnwindContext;
        void *      _pFrameInfoChain;
        unsigned __int64     _ImageBase;
        unsigned __int64     _TargetGp;
        unsigned __int64     _ThrowImageBase;
#elif defined (_M_IX86)
        void *      _pFrameInfoChain;
#endif  /* defined (_M_IX86) */
    };
    typedef struct _tiddata * _ptiddata;

    _ptiddata __cdecl _getptd();
}

const EXCEPTION_RECORD * GetCurrentExceptionRecord()
{
    _ptiddata p = _getptd();
    return (EXCEPTION_RECORD *)p->_curexception;
}
const _CONTEXT * GetCurrentExceptionContext()
{
    _ptiddata p = _getptd();
	return (_CONTEXT*)p->_curcontext;
}

#elif !defined(_MT)


extern struct EHExceptionRecord * _pCurrentException;
extern struct _CONTEXT * _pCurrentExContext;

const EXCEPTION_RECORD * GetCurrentExceptionRecord()
{
    return (EXCEPTION_RECORD *)_pCurrentException;
}
const _CONTEXT * GetCurrentExceptionContext()
{
	return _pCurrentExContext;
}

#endif //_MT
