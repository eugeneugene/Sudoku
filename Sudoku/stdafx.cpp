#include "stdafx.h"

#if 0
//#if defined(_DEBUG)
#pragma warning(disable:4074)//initializers put in compiler reserved initialization area
#pragma init_seg(compiler)//global objects in this file get constructed very early on

struct CrtBreakAllocSetter {
	CrtBreakAllocSetter() {
		_CrtSetBreakAlloc(155);
	}
};

CrtBreakAllocSetter g_crtBreakAllocSetter;
#endif
