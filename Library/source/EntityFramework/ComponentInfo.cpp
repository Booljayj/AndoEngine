#include "EntityFramework/ComponentInfo.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

DEFINE_LOG_CATEGORY( LogComponent, Debug );

bool ComponentInfo::Compare( ComponentInfo const* A, ComponentInfo const* B )
{
	return A->GetID() < B->GetID();
}
