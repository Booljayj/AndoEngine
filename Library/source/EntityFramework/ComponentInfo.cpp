#include "EntityFramework/ComponentInfo.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

bool ComponentInfo::Compare( const ComponentInfo* A, const ComponentInfo* B )
{
	return A->GetID() < B->GetID();
}

DESCRIPTION( ComponentInfo )
{
	return l_printf( CTX.Temp, "[ComponentInfo]{ ID: %i, Name: %s, Used: %i/%i }",
		Value.ID, Value.Name, Value.GetManager()->CountUsed(), Value.GetManager()->CountTotal() );
}
