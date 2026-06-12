/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Хелперы для виндового клипборда

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

#if defined(_WIN32) || defined(WIN32)

    //#include <winsock2.h>

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>

#endif

#include <string>

//----------------------------------------------------------------------------



#if defined(WIN32) || defined(_WIN32)
//----------------------------------------------------------------------------
namespace umba {
namespace win32 {

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
HWND clipboardGetConsoleHwnd()
{
    const std::size_t bufSize = 1024; // Buffer size for console window titles.
    HWND hwndFound;         // This is what is returned to the caller.
    char pszNewWindowTitle[bufSize]; // Contains fabricated
                                        // WindowTitle.
    WCHAR pszOldWindowTitle[bufSize]; // Contains original
                                        // WindowTitle.

    // Fetch current window title.

    ::GetConsoleTitleW(pszOldWindowTitle, bufSize);

    // Format a "unique" NewWindowTitle.

    wsprintfA(pszNewWindowTitle,"%d/%d",
                ::GetTickCount(),
                ::GetCurrentProcessId());

    // Change current window title.

    ::SetConsoleTitleA(pszNewWindowTitle);

    // Ensure window title has been updated.

    Sleep(50);

    // Look for NewWindowTitle.

    hwndFound = ::FindWindowA(NULL, pszNewWindowTitle);

    // Restore original window title.

    ::SetConsoleTitleW(pszOldWindowTitle);

    return(hwndFound);
}

//----------------------------------------------------------------------------
struct ClipboardAutoCloser
{
    ClipboardAutoCloser() {}
    ~ClipboardAutoCloser()
    {
        ::CloseClipboard();
    }
};

//----------------------------------------------------------------------------
inline
bool clipboardClear(HWND hWndNewOwner=0)
{
    if (!::OpenClipboard( hWndNewOwner ))
    {
        return false;
    }

    ClipboardAutoCloser autoCloser;

    return ::EmptyClipboard() ? true : false;
}
//----------------------------------------------------------------------------

template<typename StringType> inline
bool clipboardTextGet(StringType &text, HWND hWndNewOwner=0)
{
    typedef typename StringType::value_type     char_type;

    UINT cfText = CF_TEXT;
    if constexpr (sizeof(char_type)>sizeof(char))
    {
        cfText = CF_UNICODETEXT;
    }

    if (!IsClipboardFormatAvailable(cfText))
    {
        return false; // no data in this format
    }

    if (!::OpenClipboard( hWndNewOwner ))
    {
        return false;
    }

    ClipboardAutoCloser autoCloser;

    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclipboarddata
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setclipboarddata
    // https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats

    HGLOBAL hglb = (HGLOBAL)::GetClipboardData(cfText);
    if (!hglb)
    {
         return false;
    }

    std::size_t dataLen  = (std::size_t)GlobalSize( hglb );
    char_type* lpClpText = (char_type*)GlobalLock(hglb);
    if (!lpClpText)
    {
         return false;
    }

    std::size_t numTextChars = dataLen;

    if constexpr (sizeof(char_type)>sizeof(char))
    {
        // cfText = CF_UNICODETEXT;
        // dataLen
        // 78/2 = 39
        // 16+18+2+2 = 38
        numTextChars /= sizeof(char_type);
    }

    if (numTextChars>0 && lpClpText[numTextChars]==0)
    {
        --numTextChars; // remove Z-terminator
    }

    text.clear();
    text.assign(lpClpText, numTextChars);

    ::GlobalUnlock(hglb);

    while(!text.empty() && text.back()==(char_type)==0)
    {
        text.erase(text.size()-1);
    }

    return true;

}

//----------------------------------------------------------------------------
template<typename StringType> inline
bool clipboardTextSet(const StringType &text, HWND hWndNewOwner=0)
{
    typedef typename StringType::value_type     char_type;

    DWORD numBytes = (DWORD)(text.size()+1u);

    UINT cfText = CF_TEXT;
    if constexpr (sizeof(char_type)>sizeof(char))
    {
        cfText = CF_UNICODETEXT;
        numBytes *= sizeof(char_type);
    }

    if (!::OpenClipboard( hWndNewOwner ))
    {
        return false;
    }

    ClipboardAutoCloser autoCloser;

    HGLOBAL hglb = ::GlobalAlloc(GMEM_MOVEABLE, numBytes );
    if (!hglb)
    {
        return false;
    }

    char_type* lpGlobalData = (char_type*)::GlobalLock(hglb);

    text.copy(lpGlobalData, text.size()+1); // Копируем также и ноль
    lpGlobalData[text.size()] = 0;

    ::GlobalUnlock(hglb);

    ::EmptyClipboard();

    ::SetClipboardData(cfText, hglb);

    return true;
}


//----------------------------------------------------------------------------
template<typename ToUtfConverter> inline
bool clipboardTextGet(std::string &text, const ToUtfConverter &toUtfConverter, bool *pUtf, HWND hWndNewOwner=0)
{
    std::wstring wstr;
    if (umba::win32::clipboardTextGet(wstr, hWndNewOwner))
    {
        //text = encoding::toUtf8(wstr);
        text = toUtfConverter(wstr);
        if (pUtf)
           *pUtf = true;
    }
    else if (umba::win32::clipboardTextGet(text, hWndNewOwner))
    {
        if (pUtf)
           *pUtf = false;
    }
    else
    {
        return false;
    }

    return true;
}

//------------------------------
template<typename FromUtfConverter> inline
bool clipboardTextSet(const std::string &text, const FromUtfConverter &fromUtfConverter, bool utf, HWND hWndNewOwner=0)
{
    if (utf)
    {
        //return umba::win32::clipboardTextSet(encoding::fromUtf8(text), umba::win32::clipboardGetConsoleHwnd());
        return umba::win32::clipboardTextSet(fromUtfConverter(text), hWndNewOwner);
    }
    else
    {
        return umba::win32::clipboardTextSet(text, hWndNewOwner);
    }
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace win32
} // namespace umba

#endif // defined(WIN32) || defined(_WIN32)
