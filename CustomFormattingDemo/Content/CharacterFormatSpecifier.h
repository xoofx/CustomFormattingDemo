#pragma once

enum class UnderlineType
{
    None = 0,
    Single = 1,
    Double = 2,
    Triple = 3,
    Squiggly
};

enum class BackgroundMode
{
    TextHeight,
    TextHeightWithLineGap,
    LineHeight
};

class CharacterFormatSpecifier : IUnknown
{
public:
    // IUnknown methods
    virtual ULONG STDMETHODCALLTYPE AddRef() override;
    virtual ULONG STDMETHODCALLTYPE Release() override;
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                     void **ppvObject) override;

    // Public Set and Get methods
    // --------------------------

    // Foreground brush
    static HRESULT SetForegroundBrush(IDWriteTextLayout * textLayout,
                                      ID2D1Brush * brush,
                                      DWRITE_TEXT_RANGE textRange);

    ID2D1Brush * GetForegroundBrush() 
    { 
        return m_foregroundBrush.Get(); 
    }

    // Background brush
    static HRESULT SetBackgroundBrush(IDWriteTextLayout * textLayout,
                                      BackgroundMode backgroundMode,
                                      ID2D1Brush * brush,
                                      DWRITE_TEXT_RANGE textRange);

    void GetBackgroundBrush(BackgroundMode * pMode, ID2D1Brush ** pBrush)
    {
        * pMode = m_backgroundMode;
        * pBrush = m_backgroundBrush.Get();
    }

    // Underline
    static HRESULT SetUnderline(IDWriteTextLayout * textLayout,
                                UnderlineType type,
                                ID2D1Brush * brush,
                                DWRITE_TEXT_RANGE textRange);

    void GetUnderline(UnderlineType * pType, ID2D1Brush ** pBrush)
    { 
        * pType = m_underlineType; 
        * pBrush = m_underlineBrush.Get(); 
    }

    // Strikethrough
    static HRESULT SetStrikethrough(IDWriteTextLayout * textLayout,
                                    int count,
                                    ID2D1Brush * brush,
                                    DWRITE_TEXT_RANGE textRange);

    void GetStrikethrough(int * pCount, ID2D1Brush ** pBrush) 
    { 
        * pCount = m_strikethroughCount;
        * pBrush = m_strikethroughBrush.Get(); 
    }

    // Overline
    static HRESULT SetOverline(IDWriteTextLayout * textLayout,
                               bool hasOverline,
                               ID2D1Brush * brush,
                               DWRITE_TEXT_RANGE textRange);

    void GetOverline(bool * pHasOverline, ID2D1Brush ** pBrush)
    {
        *pHasOverline = m_hasOverline;
        *pBrush = m_overlineBrush.Get();
    }

    // Hightlight
    static HRESULT SetHighlight(IDWriteTextLayout * textLayout,
        ID2D1Brush * brush,
        DWRITE_TEXT_RANGE textRange);

    ID2D1Brush * GetHighlight()
    {
        return m_highlightBrush.Get();
    }


protected:
    CharacterFormatSpecifier();             // constructor
    CharacterFormatSpecifier * Clone();

    static HRESULT SetFormatting(IDWriteTextLayout * textLayout,
                                 DWRITE_TEXT_RANGE textRange,
        std::function<void(CharacterFormatSpecifier *)> setField);

private:
    LONG m_refCount;

    Microsoft::WRL::ComPtr<ID2D1Brush> m_foregroundBrush;

    BackgroundMode                     m_backgroundMode;
    Microsoft::WRL::ComPtr<ID2D1Brush> m_backgroundBrush;

    UnderlineType                      m_underlineType;
    Microsoft::WRL::ComPtr<ID2D1Brush> m_underlineBrush;

    int                                m_strikethroughCount;
    Microsoft::WRL::ComPtr<ID2D1Brush> m_strikethroughBrush;

    bool                               m_hasOverline;
    Microsoft::WRL::ComPtr<ID2D1Brush> m_overlineBrush;

    Microsoft::WRL::ComPtr<ID2D1Brush> m_highlightBrush;
};

