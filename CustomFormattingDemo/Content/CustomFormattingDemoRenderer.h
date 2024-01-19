#pragma once

#include <string>
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "CharacterFormatter.h"

namespace CustomFormattingDemo
{
    class CustomFormattingDemoRenderer
    {
    public:
        CustomFormattingDemoRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        void CreateDeviceDependentResources();
        void ReleaseDeviceDependentResources();
        void Update(DX::StepTimer const& timer);
        void Render();

    private:
        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        // Resources related to text rendering.
        std::wstring                                    m_text;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_blackBrush;
        Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock>  m_stateBlock;
        Microsoft::WRL::ComPtr<IDWriteTextLayout>       m_textLayout;
        DWRITE_TEXT_METRICS                             m_textMetrics;

        Microsoft::WRL::ComPtr<CharacterFormatter>      m_characterFormatter;
    };
}