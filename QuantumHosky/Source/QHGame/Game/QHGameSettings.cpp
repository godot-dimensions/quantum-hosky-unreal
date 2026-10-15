#include "QHGameSettings.h"

////////////////////////////////////////////////////////////////////////////////
UQHGameSettings* UQHGameSettings::Get()
{
	return GetMutableDefault<UQHGameSettings>();
}
