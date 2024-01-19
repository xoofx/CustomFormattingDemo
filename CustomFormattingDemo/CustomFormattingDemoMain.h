#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\CustomFormattingDemoRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace CustomFormattingDemo
{
	class CustomFormattingDemoMain : public DX::IDeviceNotify
	{
	public:
		CustomFormattingDemoMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~CustomFormattingDemoMain();
		void CreateWindowSizeDependentResources();
		void StartTracking() {  }
		void TrackingUpdate(float positionX) { m_pointerLocationX = positionX; }
		void StopTracking() {  }
		bool IsTracking() { return false; }
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Content renderers.
		std::unique_ptr<CustomFormattingDemoRenderer> m_customFormattingDemoRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
	};
}