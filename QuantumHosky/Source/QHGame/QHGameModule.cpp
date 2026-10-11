#include "QHGameModule.h"

#include "Modules/ModuleManager.h"

class FQHGameModule : public FDefaultGameModuleImpl
{
public:
	void StartupModule() override
	{
#if WITH_EDITOR
		FModuleManager::LoadModuleChecked<IModuleInterface>("Unreal4DCore");
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FQHGameModule, QHGame, "QHGame");

DEFINE_LOG_CATEGORY(LogQHGame);
