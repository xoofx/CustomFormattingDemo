#include "pch.h"
#include "CharacterFormatter.h"

using namespace D2D1;
using namespace Microsoft::WRL;

// Constructor
CharacterFormatter::CharacterFormatter() :
    m_refCount(0),
    m_dpiTransform(Matrix3x2F::Identity()),
    m_renderTransform(Matrix3x2F::Identity()),
    m_worldToPixel(Matrix3x2F::Identity()),
    m_pixelToWorld(Matrix3x2F::Identity())
{
}

// Custom Draw method
HRESULT CharacterFormatter::Draw(ID2D1RenderTarget * renderTarget,
                                 IDWriteTextLayout * textLayout,
                                 D2D1_POINT_2F origin,
                                 ID2D1Brush * defaultBrush)
{
    // Get the line metrics of the IDWriteTextLayout
    HRESULT hr;
    UINT32 actualLineCount;

    if (E_NOT_SUFFICIENT_BUFFER != 
        (hr = textLayout->GetLineMetrics(nullptr, 
                                         0, 
                                         &actualLineCount)))
    {
        return hr;
    }

    m_lineMetrics = std::vector<DWRITE_LINE_METRICS>(actualLineCount);

    if (S_OK != (hr = textLayout->GetLineMetrics(m_lineMetrics.data(),
                                                 m_lineMetrics.size(),
                                                 &actualLineCount)))
    {
        return hr;
    }

    m_renderTarget = renderTarget;
    m_defaultBrush = defaultBrush;

    for (m_renderPass = RenderPass::Initial;
        m_renderPass <= RenderPass::Final;
        m_renderPass = (RenderPass)((int)m_renderPass + 1))
    {
        m_lineIndex = 0;
        m_charIndex = 0;
        HRESULT hr = textLayout->Draw(nullptr, this, origin.x, origin.y);

        if (hr != S_OK)
        {
            return hr;
        }
    }
    return S_OK;
}

// IUnknown methods
ULONG STDMETHODCALLTYPE CharacterFormatter::AddRef()
{
    m_refCount++;
    return m_refCount;
}

ULONG STDMETHODCALLTYPE CharacterFormatter::Release()
{
    m_refCount--;
    LONG newCount = m_refCount;

    if (m_refCount == 0)
        delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE CharacterFormatter::QueryInterface(_In_ REFIID riid,
                                                             _Outptr_ void** ppOutput)
{
    *ppOutput = nullptr;
    HRESULT hr = S_OK;

    if (riid == __uuidof(IDWriteTextRenderer))
    {
        *ppOutput = static_cast<IDWriteTextRenderer*>(this);
        AddRef();
    }
    else if (riid == __uuidof(IDWritePixelSnapping))
    {
        *ppOutput = static_cast<IDWritePixelSnapping*>(this);
        AddRef();
    }
    else if (riid == __uuidof(IUnknown))
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

// IDWritePixelSnapping methods
HRESULT CharacterFormatter::IsPixelSnappingDisabled(void * clientDrawingContext,
    _Out_ BOOL * isDisabled)
{
    *isDisabled = false;
    return S_OK;
}

HRESULT CharacterFormatter::GetPixelsPerDip(void * clientDrawingContext,
                                            _Out_ FLOAT * pixelsPerDip)
{
    float dpiX, dpiY;
    m_renderTarget->GetDpi(&dpiX, &dpiY);
    *pixelsPerDip = dpiX / 96;

    // Save DPI as transform for pixel snapping
    m_dpiTransform = Matrix3x2F::Scale(dpiX / 96.0f, dpiY / 96.0f);
    m_worldToPixel = m_renderTransform * m_dpiTransform;
    m_pixelToWorld = m_worldToPixel;
    m_pixelToWorld.Invert();

    return S_OK;
}

HRESULT CharacterFormatter::GetCurrentTransform(void * clientDrawingContext,
                                                DWRITE_MATRIX * transform)
{
    // Matrix structures are defined identically
    m_renderTarget->GetTransform((D2D1_MATRIX_3X2_F *) transform);

    // Save transform for pixel snapping
    m_renderTransform = *(Matrix3x2F *) transform;
    m_worldToPixel = m_renderTransform * m_dpiTransform;
    m_pixelToWorld = m_worldToPixel;
    m_pixelToWorld.Invert();

    return S_OK;
}

// IDWriteTextRenderer methods
HRESULT CharacterFormatter::DrawGlyphRun(void * clientDrawingContext,
                                         FLOAT baselineOriginX,
                                         FLOAT baselineOriginY,
                                         DWRITE_MEASURING_MODE measuringMode,
                                         _In_ const DWRITE_GLYPH_RUN * glyphRun,
                                         _In_ const DWRITE_GLYPH_RUN_DESCRIPTION *
                                             glyphRunDescription,
                                         IUnknown * clientDrawingEffect)
{
    ID2D1Brush * foregroundBrush = m_defaultBrush.Get();

    BackgroundMode backgroundMode = BackgroundMode::TextHeight;
    ID2D1Brush * backgroundBrush = nullptr;
    ID2D1Brush * highlightBrush = nullptr;

    // Get foreground, background, highlight brushes
    CharacterFormatSpecifier * specifier =
        (CharacterFormatSpecifier *)clientDrawingEffect;

    if (specifier != nullptr)
    {
        specifier->GetBackgroundBrush(&backgroundMode, &backgroundBrush);

        ID2D1Brush * brush = specifier->GetForegroundBrush();

        if (brush != nullptr)
        {
            foregroundBrush = brush;
        }

        highlightBrush = specifier->GetHighlight();
    }

    // Set variable indicating trailing white space
    bool isTrailingWhiteSpace = false;

    DWRITE_LINE_METRICS lineMetrics = m_lineMetrics.at(m_lineIndex);
    UINT32 length = lineMetrics.length;

    if (length - m_charIndex == lineMetrics.trailingWhitespaceLength)
    {
        isTrailingWhiteSpace = true;
    }

    switch (m_renderPass)
    {
        case RenderPass::Initial:
        {
            if (backgroundBrush != nullptr && !isTrailingWhiteSpace)
            {
                D2D1_RECT_F rect = GetRectangle(glyphRun, 
                                                &lineMetrics,
                                                baselineOriginX, 
                                                baselineOriginY,
                                                backgroundMode);

                m_renderTarget->FillRectangle(rect, backgroundBrush);
            }
            break;
        }

        case RenderPass::Main:
        {
            m_renderTarget->DrawGlyphRun(Point2F(baselineOriginX, 
                                                 baselineOriginY),
                                         glyphRun, 
                                         foregroundBrush, 
                                         measuringMode);
            break;
        }

        case RenderPass::Final:
        {
            if (highlightBrush != nullptr && !isTrailingWhiteSpace)
            {
                D2D1_RECT_F rect = GetRectangle(glyphRun,
                                                &lineMetrics,
                                                baselineOriginX,
                                                baselineOriginY,
                                                BackgroundMode::TextHeight);

                m_renderTarget->FillRectangle(rect, highlightBrush);
            }
            break;
        }
    }

    // Increment the indices for this glyph run
    m_charIndex += glyphRunDescription->stringLength;

    if (m_charIndex == lineMetrics.length)
    {
        m_lineIndex++;
        m_charIndex = 0;
    }

    return S_OK;
}

D2D1_RECT_F CharacterFormatter::GetRectangle(const DWRITE_GLYPH_RUN * glyphRun,
                                             const DWRITE_LINE_METRICS * lineMetrics,
                                             FLOAT baselineOriginX,
                                             FLOAT baselineOriginY,
                                             BackgroundMode backgroundMode)
{
    // Get width of text
    float totalWidth = 0;

    for (UINT32 index = 0; index < glyphRun->glyphCount; index++)
    {
        totalWidth += glyphRun->glyphAdvances[index];
    }

    // Get height of text
    float ascent;
    float descent;

    if (backgroundMode == BackgroundMode::LineHeight)
    {
        ascent = lineMetrics->baseline;
        descent = lineMetrics->height - ascent;
    }
    else
    {
        DWRITE_FONT_METRICS fontMetrics;
        glyphRun->fontFace->GetMetrics(&fontMetrics);
        float adjust = glyphRun->fontEmSize / fontMetrics.designUnitsPerEm;
        ascent = adjust * fontMetrics.ascent;
        descent = adjust * fontMetrics.descent;

        if (backgroundMode == BackgroundMode::TextHeightWithLineGap)
        {
            descent += adjust * fontMetrics.lineGap;
        }
    }

    // Create rectangle
    return RectF(baselineOriginX,
                 baselineOriginY - ascent,
                 baselineOriginX + totalWidth,
                 baselineOriginY + descent);
}

HRESULT CharacterFormatter::DrawUnderline(void * clientDrawingContext,
                                          FLOAT baselineOriginX,
                                          FLOAT baselineOriginY,
                                          _In_ const DWRITE_UNDERLINE *
                                              underline,
                                          IUnknown * clientDrawingEffect)
{
    if (m_renderPass != RenderPass::Main)
    {
        return S_OK;
    }

    ID2D1Brush * underlineBrush = m_defaultBrush.Get();
    ID2D1Brush * overlineBrush = m_defaultBrush.Get();

    // Get underline count, overline boolean, and brush
    CharacterFormatSpecifier * specifier =
        (CharacterFormatSpecifier *) clientDrawingEffect;

    UnderlineType underlineType = UnderlineType::None;
    int underlineCount = 0;
    bool hasOverline = false;

    if (specifier != nullptr)
    {
        // Check for underline first
        ID2D1Brush * brush;
        specifier->GetUnderline(&underlineType, &brush);

        if (brush != nullptr)
        {
            underlineBrush = brush;
        }
        else
        {
            brush = specifier->GetForegroundBrush();

            if (brush != nullptr)
            {
                underlineBrush = brush;
            }
        }

        // Check for overline
        specifier->GetOverline(&hasOverline, &brush);

        if (brush != nullptr)
        {
            overlineBrush = brush;
        }
        else
        {
            brush = specifier->GetForegroundBrush();

            if (brush != nullptr)
            {
                overlineBrush = brush;
            }
        }
    }

    // Do squiggly underline
    if (underlineType == UnderlineType::Squiggly)
    {
        ComPtr<ID2D1Factory> factory;
        m_renderTarget->GetFactory(&factory);

        HRESULT hr;
        ComPtr<ID2D1PathGeometry> pathGeometry;

        if (S_OK != (hr = factory->CreatePathGeometry(&pathGeometry)))
            return hr;

        ComPtr<ID2D1GeometrySink> geometrySink;
        if (S_OK != (hr = pathGeometry->Open(&geometrySink)))
            return hr;

        float amplitude = 1 * underline->thickness;
        float period = 5 * underline->thickness;
        float xOffset = baselineOriginX;
        float yOffset = baselineOriginY + underline->offset;

        for (float t = 0; t <= underline->width; t++)
        {
            float x = xOffset + t;
            float angle = DirectX::XM_2PI * std::fmod(x, period) / period;
            float y = yOffset + amplitude * DirectX::XMScalarSin(angle);
            D2D1_POINT_2F pt = Point2F(x, y);

            if (t == 0)
                geometrySink->BeginFigure(pt, D2D1_FIGURE_BEGIN_HOLLOW);
            else
                geometrySink->AddLine(pt);
        }

        geometrySink->EndFigure(D2D1_FIGURE_END_OPEN);

        if (S_OK != (hr = geometrySink->Close()))
            return hr;
        
        m_renderTarget->DrawGeometry(pathGeometry.Get(), 
                                   underlineBrush, 
                                   underline->thickness);
    }
    else
    {
        underlineCount = (int) underlineType;
    }

    // Do single, double, triple underlines
    if (underlineCount == 1 || underlineCount == 3)
    {
        FillRectangle(m_renderTarget.Get(),
                      underlineBrush,
                      baselineOriginX,
                      baselineOriginY + underline->offset,
                      underline->width,
                      underline->thickness,
                      0);
    }

    if (underlineCount == 2 || underlineCount == 3)
    {
        FillRectangle(m_renderTarget.Get(),
                      underlineBrush,
                      baselineOriginX,
                      baselineOriginY + underline->offset,
                      underline->width,
                      underline->thickness,
                      underlineCount - 1);

        FillRectangle(m_renderTarget.Get(),
                      underlineBrush,
                      baselineOriginX,
                      baselineOriginY + underline->offset,
                      underline->width,
                      underline->thickness,
                      1 - underlineCount);
    }

    // Do overline
    if (hasOverline)
    {
        FillRectangle(m_renderTarget.Get(),
            overlineBrush,
            baselineOriginX,
            baselineOriginY - underline->runHeight,
            underline->width,
            underline->thickness,
            -2);
    }

    return S_OK;
}

HRESULT CharacterFormatter::DrawStrikethrough(void * clientDrawingContext,
                                              FLOAT baselineOriginX,
                                              FLOAT baselineOriginY,
                                              _In_ const DWRITE_STRIKETHROUGH *
                                                  strikethrough,
                                              IUnknown * clientDrawingEffect)
{
    if (m_renderPass != RenderPass::Main)
    {
        return S_OK;
    }

    ID2D1Brush * foregroundBrush = m_defaultBrush.Get();

    // Get strikethrough count and brush
    CharacterFormatSpecifier * specifier =
        (CharacterFormatSpecifier *) clientDrawingEffect;

    int strikethroughCount = 0;

    if (specifier != nullptr)
    {
        ID2D1Brush * brush;
        specifier->GetStrikethrough(&strikethroughCount, &brush);

        if (brush != nullptr)
        {
            foregroundBrush = brush;
        }
        else
        {
            brush = specifier->GetForegroundBrush();

            if (brush != nullptr)
            {
                foregroundBrush = brush;
            }
        }
    }

    if (strikethroughCount < 0 || strikethroughCount > 3)
        return E_INVALIDARG;

    if (strikethroughCount == 1 || strikethroughCount == 3)
    {
        FillRectangle(m_renderTarget.Get(),
                      foregroundBrush,
                      baselineOriginX,
                      baselineOriginY + strikethrough->offset,
                      strikethrough->width,
                      strikethrough->thickness,
                      0);
    }
    if (strikethroughCount == 2 || strikethroughCount == 3)
    {
        FillRectangle(m_renderTarget.Get(),
                      foregroundBrush,
                      baselineOriginX,
                      baselineOriginY + strikethrough->offset,
                      strikethrough->width,
                      strikethrough->thickness,
                      strikethroughCount - 1);

        FillRectangle(m_renderTarget.Get(),
                      foregroundBrush,
                      baselineOriginX,
                      baselineOriginY + strikethrough->offset,
                      strikethrough->width,
                      strikethrough->thickness,
                      1 - strikethroughCount);
    }
    return S_OK;
}

HRESULT CharacterFormatter::DrawInlineObject(void * clientDrawingContext,
                                             FLOAT originX,
                                             FLOAT originY,
                                             IDWriteInlineObject * inlineObject,
                                             BOOL isSideways,
                                             BOOL isRightToLeft,
                                             IUnknown * clientDrawingEffect)
{
    if (m_renderPass != RenderPass::Main)
    {
        return S_OK;
    }

    return inlineObject->Draw(clientDrawingContext,
                              this,
                              originX,
                              originY,
                              isSideways,
                              isRightToLeft,
                              clientDrawingEffect);
}

void CharacterFormatter::FillRectangle(ID2D1RenderTarget * renderTarget,
                                       ID2D1Brush * brush,
                                       float x, float y,
                                       float width, float thickness,
                                       int offset)
{
    // Snap the y coordinate to the nearest pixel
    D2D1_POINT_2F pt = Point2F(0, y);
    pt = m_worldToPixel.TransformPoint(pt);
    pt.y = (float) (int) (pt.y + 0.5f);
    pt = m_pixelToWorld.TransformPoint(pt);
    y = pt.y;

    // Adjust for spacing
    y += offset * thickness;

    // Fill the rectangle
    D2D1_RECT_F rect = RectF(x, y, x + width, y + thickness);
    renderTarget->FillRectangle(&rect, brush);
}
