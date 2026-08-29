#include "MotionForgeKimodoToolsetModule.h"

#include "KimodoToolset.h"
#include "ToolsetRegistry/UToolsetRegistry.h"

DEFINE_LOG_CATEGORY(LogKimodoToolset);

#define LOCTEXT_NAMESPACE "FMotionForgeKimodoToolsetModule"

void FMotionForgeKimodoToolsetModule::StartupModule()
{
	if (!UToolsetRegistry::IsAvailable())
	{
		// Expected outside the editor, and whenever the experimental plugins are off. Not an error.
		UE_LOG(LogKimodoToolset, Log,
			TEXT("Toolset registry unavailable - Kimodo tools not registered."));
		return;
	}

	if (UToolsetRegistry::IsToolsetClassRegistered(UKimodoToolset::StaticClass()))
	{
		bRegistered = true;
		return;
	}

	UToolsetRegistry::RegisterToolsetClass(UKimodoToolset::StaticClass());
	bRegistered = UToolsetRegistry::IsToolsetClassRegistered(UKimodoToolset::StaticClass());

	// Worth logging either way. The failure mode this replaces was silent: the module loaded, the
	// class existed, and the tools simply were not there.
	UE_LOG(LogKimodoToolset, Log, TEXT("Kimodo toolset %s."),
		bRegistered ? TEXT("registered") : TEXT("failed to register"));
}

void FMotionForgeKimodoToolsetModule::ShutdownModule()
{
	if (bRegistered && UToolsetRegistry::IsAvailable())
	{
		UToolsetRegistry::UnregisterToolsetClass(UKimodoToolset::StaticClass());
		bRegistered = false;
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMotionForgeKimodoToolsetModule, MotionForgeKimodoToolset)
