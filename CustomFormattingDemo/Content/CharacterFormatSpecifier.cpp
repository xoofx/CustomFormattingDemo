#include "pch.h"
#include "CharacterFormatSpecifier.h"

CharacterFormatSpecifier::CharacterFormatSpecifier() :
    m_refCount(0),
    m_foregroundBrush(nullptr),
    m_backgroundMode(BackgroundMode::TextHeight),
    m_backgroundBrush(nullptr),
    m_underlineType(UnderlineType::None),
    m_underlineBrush(nullptr),
    m_strikethroughCount(0),
    m_strikethroughBrush(nullptr),
    m_hasOverline(false),
    m_overlineBrush(nullptr),
    m_highlightBrush(nullptr)
{
}

// IUnknown methods
ULONG STDMETHODCALLTYPE CharacterFormatSpecifier::AddRef()
{
    m_refCount++;
    return m_refCount;
}

ULONG STDMETHODCALLTYPE CharacterFormatSpecifier::Release()
{
    m_refCount--;
    LONG newCount = m_refCount;

    if (m_refCount == 0)
        delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE CharacterFormatSpecifier::QueryInterface(_In_ REFIID riid,
    _Outptr_ void** ppOutput)
{
    *ppOutput = nullptr;
    HRESULT hr = S_OK;

    if (riid == __uuidof(IUnknown))
    {
        *ppOutput = this;
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}

CharacterFormatSpecifier * CharacterFormatSpecifier::Clone()
{
    CharacterFormatSpecifier * specifier = new CharacterFormatSpecifier();

    specifier->m_foregroundBrush = this->m_foregroundBrush;
    specifier->m_backgroundMode = this->m_backgroundMode;
    specifier->m_backgroundBrush = this->m_backgroundBrush;
    specifier->m_underlineType = this->m_underlineType;
    specifier->m_underlineBrush = this->m_underlineBrush;
    specifier->m_strikethroughCount = this->m_strikethroughCount;
    specifier->m_strikethroughBrush = this->m_strikethroughBrush;
    specifier->m_hasOverline = this->m_hasOverline;
    specifier->m_overlineBrush = this->m_overlineBrush;
    specifier->m_highlightBrush = this->m_highlightBrush;

    return specifier;
}

// All remaining methods are static!
HRESULT CharacterFormatSpecifier::SetForegroundBrush(
                                        IDWriteTextLayout * textLayout,
                                        ID2D1Brush * brush,
                                        DWRITE_TEXT_RANGE textRange)
{
    return SetFormatting(textLayout, 
                         textRange, 
                         [brush](CharacterFormatSpecifier * specifier)
    {
        specifier->m_foregroundBrush = brush;
    });
}

HRESULT CharacterFormatSpecifier::SetBackgroundBrush(
    IDWriteTextLayout * textLayout,
    BackgroundMode backgroundMode,
    ID2D1Brush * brush,
    DWRITE_TEXT_RANGE textRange)
{
    return SetFormatting(textLayout,
        textRange,
        [backgroundMode, brush](CharacterFormatSpecifier * specifier)
    {
        specifier->m_backgroundMode = backgroundMode;
        specifier->m_backgroundBrush = brush;
    });
}

HRESULT CharacterFormatSpecifier::SetUnderline(IDWriteTextLayout * textLayout,
                                               UnderlineType type,
                                               ID2D1Brush * brush,
                                               DWRITE_TEXT_RANGE textRange)
{
    textLayout->SetUnderline(true, textRange);

    return SetFormatting(textLayout, 
                         textRange, 
                         [type, brush](CharacterFormatSpecifier * specifier)
    {
        specifier->m_underlineType = type;
        specifier->m_underlineBrush = brush;
    });
}

HRESULT CharacterFormatSpecifier::SetStrikethrough(IDWriteTextLayout * textLayout,
                                                   int count,
                                                   ID2D1Brush * brush,
                                                   DWRITE_TEXT_RANGE textRange)
{
    if (count < 0 || count > 3)
    {
        return E_INVALIDARG;
    }

    textLayout->SetStrikethrough(count > 0, textRange);

    return SetFormatting(textLayout,
        textRange,
        [count, brush](CharacterFormatSpecifier * specifier)
    {
        specifier->m_strikethroughCount = count;
        specifier->m_strikethroughBrush = brush;
    });
}

HRESULT CharacterFormatSpecifier::SetOverline(IDWriteTextLayout * textLayout,
                                              bool hasOverline,
                                              ID2D1Brush * brush,
                                              DWRITE_TEXT_RANGE textRange)
{
    textLayout->SetUnderline(true, textRange);

    return SetFormatting(textLayout,
                         textRange,
                         [hasOverline, brush](CharacterFormatSpecifier * specifier)
    {
        specifier->m_hasOverline = hasOverline;
        specifier->m_overlineBrush = brush;
    });
}

HRESULT CharacterFormatSpecifier::SetHighlight(
    IDWriteTextLayout * textLayout,
    ID2D1Brush * brush,
    DWRITE_TEXT_RANGE textRange)
{
    return SetFormatting(textLayout,
        textRange,
        [brush](CharacterFormatSpecifier * specifier)
    {
        specifier->m_highlightBrush = brush;
    });
}

HRESULT CharacterFormatSpecifier::SetFormatting(IDWriteTextLayout * textLayout,
                                                DWRITE_TEXT_RANGE textRange,
                std::function<void(CharacterFormatSpecifier *)> setField)
{
    // Get information from the text range to set
    const UINT32 endPosition = textRange.startPosition + textRange.length;
    UINT32 currentPosition = textRange.startPosition;

    // Loop until we're at the end of the range
    while (currentPosition < endPosition)
    {
        // Get the drawing effect at the current position
        CharacterFormatSpecifier * specifier = nullptr;
        DWRITE_TEXT_RANGE queryTextRange;
        HRESULT hr;

        if (S_OK != (hr = textLayout->GetDrawingEffect(currentPosition, 
                                                       (IUnknown **) &specifier, 
                                                       &queryTextRange)))
        {
            return hr;
        }

        // Create a new CharacterFormatSpecifier or clone the existing one
        if (specifier == nullptr)
        {
            specifier = new CharacterFormatSpecifier();
        }
        else
        {
            specifier = specifier->Clone();
        }

        // Callback to set fields in the new CharacterFormatSpecifier!!!
        setField(specifier);

        // Determine the text range for the new CharacterFormatSpecifier
        UINT32 queryEndPos = queryTextRange.startPosition + queryTextRange.length;
        UINT32 setLength = min(endPosition, queryEndPos) - currentPosition;

        DWRITE_TEXT_RANGE setTextRange;
        setTextRange.startPosition = currentPosition;
        setTextRange.length = setLength;

        // Set it
        if (S_OK != (hr = textLayout->SetDrawingEffect((IUnknown *) specifier, 
                                                       setTextRange)))
        {
            return hr;
        }

        // Bump up the current position
        currentPosition += setLength;
    }
    return S_OK;
}