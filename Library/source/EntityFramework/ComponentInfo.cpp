#include "EntityFramework/ComponentInfo.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

bool ComponentInfo::Compare( ComponentInfo const* A, ComponentInfo const* B )
{
	return A->GetID() < B->GetID();
}

DESCRIPTION( ComponentInfo )
{
	return l_printf( CTX.Temp, "[ComponentInfo]{ ID: %i, Name: %s, Used: %i/%i }",
		Value.ID, Value.Name, Value.GetManager()->CountUsed(), Value.GetManager()->CountTotal() );
}
