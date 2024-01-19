#pragma once
#include "CharacterFormatSpecifier.h"

class CharacterFormatter : public IDWriteTextRenderer
{
public:
    CharacterFormatter();

    // Draw method
    HRESULT Draw(ID2D1RenderTarget * renderTarget,
                 IDWriteTextLayout * textLayout,
                 D2D1_POINT_2F origin,
                 ID2D1Brush * defaultBrush);

    // IUnknown methods
    virtual ULONG STDMETHODCALLTYPE AddRef() override;
    virtual ULONG STDMETHODCALLTYPE Release() override;
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;

    // IDWritePixelSnapping methods
    virtual HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void * clientDrawingContext,
                                                              _Out_ BOOL * isDisabled) override;

    virtual HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void * clientDrawingContext,
                                                      _Out_ FLOAT * pixelsPerDip) override;

    virtual HRESULT STDMETHODCALLTYPE GetCurrentTransform(void * clientDrawingContext,
                                                          _Out_ DWRITE_MATRIX * transform) override;

    // IDWriteTextRenderer methods
    virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(void * clientDrawingContext,
                                                   FLOAT baselineOriginX,
                                                   FLOAT baselineOriginY,
                                                   DWRITE_MEASURING_MODE measuringMode,
                                                   _In_ const DWRITE_GLYPH_RUN * glyphRun,
                                                   _In_ const DWRITE_GLYPH_RUN_DESCRIPTION * 
                                                                    glyphRunDescription,
                                                   IUnknown * clientDrawingEffect) override;

    virtual HRESULT STDMETHODCALLTYPE DrawUnderline(void * clientDrawingContext,
                                                    FLOAT baselineOriginX,
                                                    FLOAT baselineOriginY,
                                                    _In_ const DWRITE_UNDERLINE * underline,
                                                    IUnknown * clientDrawingEffect) override;

    virtual HRESULT STDMETHODCALLTYPE DrawStrikethrough(void * clientDrawingContext,
                                                        FLOAT baselineOriginX,
                                                        FLOAT baselineOriginY,
                                                        _In_ const DWRITE_STRIKETHROUGH * strikethrough,
                                                        IUnknown * clientDrawingEffect) override;

    virtual HRESULT STDMETHODCALLTYPE DrawInlineObject(void * clientDrawingContext,
                                                       FLOAT originX,
                                                       FLOAT originY,
                                                       IDWriteInlineObject * inlineObject,
                                                       BOOL isSideways,
                                                       BOOL isRightToLeft,
                                                       IUnknown * clientDrawingEffect) override;

private:
    LONG m_refCount;

    Microsoft::WRL::ComPtr<ID2D1RenderTarget> m_renderTarget;
    Microsoft::WRL::ComPtr<ID2D1Brush>        m_defaultBrush;

    enum class RenderPass
    {
        Initial,
        Main,
        Final
    };

    RenderPass m_renderPass;

    std::vector<DWRITE_LINE_METRICS> m_lineMetrics;
    int                              m_lineIndex;
    int                              m_charIndex;
   
    D2D1::Matrix3x2F m_dpiTransform;
    D2D1::Matrix3x2F m_renderTransform;
    D2D1::Matrix3x2F m_worldToPixel;
    D2D1::Matrix3x2F m_pixelToWorld;

    D2D1_RECT_F GetRectangle(const DWRITE_GLYPH_RUN * glyphRun,
                             const DWRITE_LINE_METRICS * lineMetrics,
                             FLOAT baselineOriginX,
                             FLOAT baselineOriginY,
                             BackgroundMode backgroundMode);

    void FillRectangle(ID2D1RenderTarget * renderTarget,
                       ID2D1Brush * brush,
                       float x, float y, 
                       float width, float thickness,
                       int offset);
};
