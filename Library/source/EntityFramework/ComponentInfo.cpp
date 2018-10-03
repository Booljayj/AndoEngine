#include "EntityFramework/ComponentInfo.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

bool ComponentInfo::Compare( ComponentInfo const* A, ComponentInfo const* B )
{
	return A->GetID() < B->GetID();
}
