#include <iostream>
#include "hand_proximity_overlay_service.h"
#include "performance_tracker.h"

int main()
{
	auto overlayService = new HandProximityOverlayService();

	overlayService->ResizeWindow(200, 200);
	PerformanceTracker perf = PerformanceTracker();
	while (true)
	{

		POINT pt;
		if (GetCursorPos(&pt))
		{
			perf.Start();

			overlayService->SetPosition(pt.x, pt.y);
			overlayService->Draw();

			perf.Stop();
		}
	}

	return 0;
}