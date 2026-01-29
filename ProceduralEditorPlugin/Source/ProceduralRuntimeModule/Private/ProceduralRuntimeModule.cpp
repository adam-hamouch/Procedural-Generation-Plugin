#include "ProceduralRuntimeModule.h"
#include "Modules/ModuleManager.h"

class FProceduralRuntimeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FProceduralRuntimeModule, ProceduralRuntimeModule)