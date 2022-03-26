#pragma once

#include <locale.h>
#include <wctype.h>

#define GCT_ALPHA				_ALPHA
#define GCT_ALNUM				(_ALPHA|_DIGIT)
#define GCT_BLANK				_BLANK
#define GCT_CONTROL				_CONTROL
#define GCT_DIGIT				_DIGIT
#define GCT_GRAPH				(_ALPHA|_DIGIT|_PUNCT)
#define GCT_LOWER				_LOWER
#define GCT_PRINT				(_ALPHA|_BLANK|_DIGIT|_PUNCT)
#define GCT_PUNCT				_PUNCT
#define GCT_BLANK				_BLANK
#define GCT_SPACE				_SPACE
#define GCT_UPPER				_UPPER
#define GCT_HEX					_HEX
#define GCT_ALL					(GCT_ALPHA|GCT_BLANK|GCT_CONTROL|GCT_DIGIT|GCT_LOWER|GCT_PUNCT|GCT_BLANK|GCT_SPACE|GCT_UPPER|GCT_HEX)

#if defined(_UNICODE)
typedef wctype_t _gctype_t;
inline bool _IsCType(wint_t _Char, _gctype_t _Type, const _locale_t& _Loc)
{
	return (bool) _iswctype_l(_Char, _Type, _Loc);
}
#else
typedef wctype_t _gctype_t;
inline bool _IsCType(int _Char, _gctype_t _Type, const _locale_t& _Loc)
{
	return (bool)_isctype_l(_Char, _Type, _Loc);
}
#endif
