#include "pch.h"
#include "CustomFormattingDemoRenderer.h"

#include "Common/DirectXHelper.h"

using namespace CustomFormattingDemo;

using namespace D2D1;
using namespace Microsoft::WRL;

CustomFormattingDemoRenderer::CustomFormattingDemoRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) : 
    m_deviceResources(deviceResources)
{
    // Create device independent resources
    DX::ThrowIfFailed(
        m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
        );

    ComPtr<IDWriteTextFormat> textFormat;

    DX::ThrowIfFailed(
        m_deviceResources->GetDWriteFactory()->CreateTextFormat(
            L"Times New Roman",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            24.0f,
            L"en-us",
            &textFormat)
        );

    m_text = L"This paragraph of text rendered with "
             L"DirectWrite is based on "
             L"IDWriteTextFormat and IDWriteTextLayout "
             L"objects and uses a custom format "
             L"specifier passed to the "
             L"SetDrawingEffect method, and thus "
             L"is capable of different "
             L"RBG RGB foreground colors, such as red, "
             L"green, and blue as well as double "
             L"underline, triple underline, double "
             L"strikethrough, triple strikethrough, "
             L"or combinations thereof. Also possible "
             L"is something often referred to as an "
             L"overline, as well as a squiggly "
             L"(squiggly?) underline.";

    DX::ThrowIfFailed(
        m_deviceResources->GetDWriteFactory()->CreateTextLayout(
            m_text.c_str(),
            (uint32) m_text.length(),
            textFormat.Get(),
            440.0f,         // Max width for paragraph
            std::numeric_limits<float>::infinity(),
            &m_textLayout)
        );

    // Set character formatting
    DWRITE_TEXT_RANGE textRange;

    std::wstring strFind = L"IDWriteTextFormat";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();
    DX::ThrowIfFailed(
        m_textLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, textRange)
        );

    strFind = L"IDWriteTextLayout";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();
    DX::ThrowIfFailed(
        m_textLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, textRange)
        );

    strFind = L"SetDrawingEffect";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();
    DX::ThrowIfFailed(
        m_textLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, textRange)
        );

    // Get text metrics
    DX::ThrowIfFailed(
        m_textLayout->GetMetrics(&m_textMetrics)
        );

    // Instantiate CharacterFormatter
    m_characterFormatter = new CharacterFormatter();

    CreateDeviceDependentResources();
}

void CustomFormattingDemoRenderer::CreateDeviceDependentResources()
{
    // Create brushes and set them
    ID2D1DeviceContext1* context = m_deviceResources->GetD2DDeviceContext();

    ComPtr<ID2D1SolidColorBrush> redBrush;
    DX::ThrowIfFailed(
        context->CreateSolidColorBrush(ColorF(ColorF::Red), &redBrush)
        );

    ComPtr<ID2D1SolidColorBrush> greenBrush;
    DX::ThrowIfFailed(
        context->CreateSolidColorBrush(ColorF(ColorF::Green), &greenBrush)
        );

    ComPtr<ID2D1SolidColorBrush> blueBrush;
    DX::ThrowIfFailed(
        context->CreateSolidColorBrush(ColorF(ColorF::Blue), &blueBrush)
        );

    DWRITE_TEXT_RANGE textRange;
    std::wstring strFind = L"RBG";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetStrikethrough(m_textLayout.Get(),
                                                   1,
                                                   nullptr,
                                                   textRange)
        );

    // Individual letters after strikethrough
    textRange.length = 1;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     redBrush.Get(),
                                                     textRange)
        );

    textRange.startPosition += 1;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     blueBrush.Get(),
                                                     textRange)
        );

    textRange.startPosition += 1;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     greenBrush.Get(),
                                                     textRange)
        );

    strFind = L"RGB";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    // Individual letters before underline
    textRange.length = 1;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     redBrush.Get(),
                                                     textRange)
        );

    textRange.startPosition += 1;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     greenBrush.Get(),
                                                     textRange)
        );

    textRange.startPosition += 1;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     blueBrush.Get(),
                                                     textRange)
        );

    textRange.startPosition -= 2;
    textRange.length = 3;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetUnderline(m_textLayout.Get(),
                                               UnderlineType::Single,
                                               redBrush.Get(),
                                               textRange)
        );

    strFind = L" red";      // avoid "rendered"
    textRange.startPosition = m_text.find(strFind.data()) + 1;
    textRange.length = strFind.length() - 1;
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     redBrush.Get(),
                                                     textRange)
        );

    strFind = L"green";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     greenBrush.Get(),
                                                     textRange)
        );

    strFind = L"blue";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();
    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetForegroundBrush(m_textLayout.Get(),
                                                     blueBrush.Get(),
                                                     textRange)
        );

    // Set custom underlining and strikethrough
    strFind = L"double underline";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetUnderline(m_textLayout.Get(),
                                               UnderlineType::Double,
                                               redBrush.Get(),
                                               textRange)
        );

    strFind = L"triple underline";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetUnderline(m_textLayout.Get(),
                                               UnderlineType::Triple,
                                               nullptr,
                                               textRange)
        );

    strFind = L"double strikethrough";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetStrikethrough(m_textLayout.Get(),
                                                   2,
                                                   nullptr,
                                                   textRange)
        );

    strFind = L"triple strikethrough";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetStrikethrough(m_textLayout.Get(),
                                                   3,
                                                   nullptr,
                                                   textRange)
        );

    strFind = L"combinations";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetUnderline(m_textLayout.Get(),
                                               UnderlineType::Triple,
                                               nullptr,
                                               textRange)
        );

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetStrikethrough(m_textLayout.Get(),
                                                   2,
                                                   redBrush.Get(),
                                                   textRange)
        );

    strFind = L"thereof";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetUnderline(m_textLayout.Get(),
                                               UnderlineType::Double,
                                               blueBrush.Get(),
                                               textRange)
        );

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetStrikethrough(m_textLayout.Get(),
                                                   3,
                                                   nullptr,
                                                   textRange)
        );

    strFind = L"overline";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetOverline(m_textLayout.Get(),
                                              true,
                                              blueBrush.Get(),
                                              textRange)
        );

    strFind = L"squiggly (squiggly?) underline";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetUnderline(m_textLayout.Get(),
                                               UnderlineType::Squiggly,
                                               blueBrush.Get(),
                                               textRange)
        );

    strFind = L"(squiggly?)";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        m_textLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, textRange)
        );

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetUnderline(m_textLayout.Get(),
                                               UnderlineType::Squiggly,
                                               redBrush.Get(),
                                               textRange)
        );

    // Set background brush
    ComPtr<ID2D1SolidColorBrush> magentaBrush;

    DX::ThrowIfFailed(
        context->CreateSolidColorBrush(ColorF(ColorF::Magenta), 
                                              &magentaBrush)
        );

    strFind = L"IDWriteTextFormat and IDWriteTextLayout objects";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetBackgroundBrush(m_textLayout.Get(),
                                                     BackgroundMode::LineHeight,
                                                     magentaBrush.Get(), 
                                                     textRange)
        );

    // Set highlight brush
    ComPtr<ID2D1SolidColorBrush> highlightBrush;

    DX::ThrowIfFailed(
        context->CreateSolidColorBrush(ColorF(1.0f, 1.0f, 0, 0.5f),
                                       &highlightBrush)
        );

    strFind = L"the SetDrawingEffect method";
    textRange.startPosition = m_text.find(strFind.data());
    textRange.length = strFind.length();

    DX::ThrowIfFailed(
        CharacterFormatSpecifier::SetHighlight(m_textLayout.Get(),
                                               highlightBrush.Get(),
                                               textRange)
        );

    // Create brush for default text 
    DX::ThrowIfFailed(
        context->CreateSolidColorBrush(ColorF(ColorF::Black), &m_blackBrush)
        );
}
void CustomFormattingDemoRenderer::ReleaseDeviceDependentResources()
{
    m_blackBrush.Reset();
}

// Updates the text to be displayed.
void CustomFormattingDemoRenderer::Update(DX::StepTimer const& timer)
{
}

// Renders a frame to the screen.
void CustomFormattingDemoRenderer::Render()
{
    ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
    Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

    context->SaveDrawingState(m_stateBlock.Get());
    context->BeginDraw();
    context->Clear(ColorF(ColorF::AliceBlue));

    // Center text on the screen
    Matrix3x2F screenTranslation = Matrix3x2F::Translation(
        (logicalSize.Width - m_textLayout->GetMaxWidth()) / 2,
        (logicalSize.Height - m_textMetrics.height) / 2);

    context->SetTransform(screenTranslation *
        m_deviceResources->GetOrientationTransform2D());

    // Display paragraph of text with custom text renderer
    D2D1_POINT_2F origin = Point2F();

    DX::ThrowIfFailed(
        m_characterFormatter->Draw(context, 
                                   m_textLayout.Get(), 
                                   origin, 
                                   m_blackBrush.Get())
        );

    // Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    HRESULT hr = context->EndDraw();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        DX::ThrowIfFailed(hr);
    }

    context->RestoreDrawingState(m_stateBlock.Get());
}

