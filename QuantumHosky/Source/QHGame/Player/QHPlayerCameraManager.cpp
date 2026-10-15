#include "QHPlayerCameraManager.h"

////////////////////////////////////////////////////////////////////////////////
const FMinimalViewInfo& AQHPlayerCameraManager::GetViewInfo() const
{
	return GetCameraCacheView();
}
