#include "pch.h"
#include "CustomFormattingDemoMain.h"
#include "Common\DirectXHelper.h"

using namespace CustomFormattingDemo;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
CustomFormattingDemoMain::CustomFormattingDemoMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), m_pointerLocationX(0.0f)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// Content initialization.
	m_customFormattingDemoRenderer = std::unique_ptr<CustomFormattingDemoRenderer>(new CustomFormattingDemoRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

CustomFormattingDemoMain::~CustomFormattingDemoMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void CustomFormattingDemoMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
}

void CustomFormattingDemoMain::StartRenderLoop()
{
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Create a task that will be run on a background thread.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Calculate the updated frame and render once per vertical blanking interval.
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			Update();
			if (Render())
			{
				m_deviceResources->Present();
			}
		}
	});

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void CustomFormattingDemoMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

// Updates the application state once per frame.
void CustomFormattingDemoMain::Update() 
{
	ProcessInput();

	// Update scene objects.
	m_timer.Tick([&]()
	{
		// Content update functions.
		m_customFormattingDemoRenderer->Update(m_timer);
	});
}

// Process all input from the user before updating game state
void CustomFormattingDemoMain::ProcessInput()
{
	// TODO: Add per frame input handling here.
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool CustomFormattingDemoMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}


	// Render the scene objects.
	// Content rendering functions.
	m_customFormattingDemoRenderer->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void CustomFormattingDemoMain::OnDeviceLost()
{
	m_customFormattingDemoRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void CustomFormattingDemoMain::OnDeviceRestored()
{
	m_customFormattingDemoRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
