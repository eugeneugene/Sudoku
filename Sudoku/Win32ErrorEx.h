//////////////////////////////////////////////////////////////////////
//
// Win32ErrorEx.h:
// Based on CWin32Error class by Ajit Jadhav (ajit_jadhav@hotmail.com)
//
//////////////////////////////////////////////////////////////////////
// Win32Error.h:    
//      Interface and Implementation for the 
//      CWin32Error class.
//
//      Written by and copyright (c) 2001 Ajit Jadhav.
//      All rights reserved by the author.
//
// Legal Notice:
// * The material in this source file can be used for any 
// purpose, commercial or noncommercial, without paying any 
// charge.
// * However, use this material, in full or in part, at your
// own risk. The author assumes no responsibility--implied 
// or otherwise--arising from any direct or indirect damages 
// which may result by using the material presented here. 
// * If you decide to use or reuse this material, please make
// sure to keep the this copyright notice intact in your 
// source-files.
// * However, it is NOT necessary to acknowledge the copyright 
// outside of your project source files. (For example, it is 
// NOT necessary to pop up a message box acknowledging this 
// copyright in your application.)
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>
#include <memory.h>
#include <stdio.h>

template <class _Type> class CWin32ErrorBase
{
public:
	CWin32ErrorBase() : m_szErrMsg(nullptr) {}
	const CWin32ErrorBase& operator=(const CWin32ErrorBase& rRHS)
	{
		if (this != &rRHS)
		{
			releaseRef();
			addRef(rRHS);
		}
		return *this;
	}
	~CWin32ErrorBase()
	{
		releaseRef();
	}
	unsigned int ErrorCode() const
	{
		return metaMem() ? metaMem()->m_dwErrCode : -1;
	}
	operator unsigned int() const
	{
		return metaMem() ? metaMem()->m_dwErrCode : -1;
	}
	int MessageLength() const
	{
		return metaMem() ? metaMem()->m_nMsgStrLen : 0;
	}
	operator const _Type* () const
	{
		return m_szErrMsg;
	}
	const _Type* Description() const
	{
		return m_szErrMsg;
	}
	void ReGetLastError()
	{
		unsigned int dwNewErr = ::GetLastError();
		if (dwNewErr != metaMem()->m_dwErrCode)
		{
			releaseRef();
			doFormatMessage(dwNewErr);
		}
	}

protected:
	virtual void doFormatMessage(unsigned int dwLastErr) = 0;
	struct SMetaMem
	{
		SMetaMem() : m_nMsgStrLen{ 0 }, m_dwErrCode{ 0 }, m_nRefCnt{ 0 } {}

		long m_nRefCnt;
		unsigned int m_dwErrCode;
		int m_nMsgStrLen;

		_Type* getString() { return (_Type*)(this + 1); }
	};
	void addRef(const CWin32ErrorBase& rOrig)
	{
		::InterlockedIncrement(&rOrig.metaMem()->m_nRefCnt);
		m_szErrMsg = rOrig.m_szErrMsg;
	}
	void releaseRef()
	{
		if (!metaMem())
			return;
		if (!::InterlockedDecrement(&metaMem()->m_nRefCnt))
			freeBuffer();
	}
	void allocCopyData(const _Type* pTemp, int nLen, unsigned int dwLastErr)
	{
		SMetaMem* pSM = nullptr;
		pSM = (SMetaMem*) new unsigned char[sizeof(SMetaMem) + (nLen + 1) * sizeof(_Type)];
		if (!pSM)
			return;
		pSM->m_dwErrCode = dwLastErr;
		pSM->m_nRefCnt = 1;
		pSM->m_nMsgStrLen = nLen;
		m_szErrMsg = pSM->getString();
		::memcpy(m_szErrMsg, pTemp, nLen * sizeof(_Type));
		m_szErrMsg[nLen] = NULL;
	}
	void freeBuffer()
	{
		if (metaMem())
		{
			delete[](unsigned char*) metaMem();
			m_szErrMsg = nullptr;
		}
	}
	SMetaMem* metaMem() const
	{
		return m_szErrMsg ? (SMetaMem*)m_szErrMsg - 1 : nullptr;
	}
	_Type* m_szErrMsg;
};

class CWin32ErrorA : public CWin32ErrorBase<char>
{
	typedef public CWin32ErrorBase<char> _Base;

public:
	CWin32ErrorA() : _Base() { doFormatMessage(::GetLastError()); }
	CWin32ErrorA(const CWin32ErrorA& rRHS) : _Base() { addRef(rRHS); }
	CWin32ErrorA(unsigned int dwLastError) : _Base() { doFormatMessage(dwLastError); }

private:
	void doFormatMessage(unsigned int dwLastErr) override
	{
		m_szErrMsg = nullptr;

		char* pTemp = nullptr;
		int nLen = ::FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwLastErr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(char*)&pTemp,
			1,
			nullptr);
		if (nLen)
		{
			allocCopyData(pTemp, nLen, dwLastErr);
			::LocalFree(pTemp);
		}
		else
		{
			DWORD err = ::GetLastError();
			std::stringstream buf;
			buf << "Unknown error: 0x" << std::hex << std::setw(2) << std::setfill('0') << err << std::endl;
			allocCopyData(buf.str().c_str(), buf.str().length(), dwLastErr);
		}
	}
};

class CWin32ErrorW : public CWin32ErrorBase<wchar_t>
{
	typedef public CWin32ErrorBase<wchar_t> _Base;

public:
	CWin32ErrorW() : _Base() { doFormatMessage(::GetLastError()); }
	CWin32ErrorW(const CWin32ErrorW& rRHS) : _Base() { addRef(rRHS); }
	CWin32ErrorW(unsigned int dwLastError) : _Base() { doFormatMessage(dwLastError); }

private:
	void doFormatMessage(unsigned int dwLastErr) override
	{
		m_szErrMsg = nullptr;

		wchar_t* pTemp = nullptr;
		int nLen = ::FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwLastErr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(wchar_t*)&pTemp,
			1,
			nullptr);
		if (nLen)
		{
			allocCopyData(pTemp, nLen, dwLastErr);
			::LocalFree(pTemp);
		}
		else
		{
			DWORD err = ::GetLastError();
			wchar_t buf[30];
			int l = swprintf_s(buf, L"Unknown error: 0x%08X\n", err);
			allocCopyData(buf, l, dwLastErr);
		}
	}
};

#if defined(_UNICODE)
typedef CWin32ErrorW CWin32ErrorT;
#else
typedef CWin32ErrorA CWin32ErrorT;
#endif

#define CWin32Error CWin32ErrorT
