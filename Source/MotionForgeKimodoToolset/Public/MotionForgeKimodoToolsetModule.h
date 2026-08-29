#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

MOTIONFORGEKIMODOTOOLSET_API DECLARE_LOG_CATEGORY_EXTERN(LogKimodoToolset, Log, All);

/**
 * Registers the Kimodo toolset with ToolsetRegistry.
 *
 * Registration is **explicit**, not by reflection. A UToolsetDefinition subclass that nobody
 * registers compiles, loads, and is invisible - the module appears in the log, the class exists in
 * memory, and no agent can see a single tool. There is nothing to say it went wrong.
 */
class FMotionForgeKimodoToolsetModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	bool bRegistered = false;
};
