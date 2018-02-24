#include "EntityFramework/ComponentInfo.h"
#include "Engine/LinearStrings.h"

DESCRIPTION( ComponentInfo )
{
	return l_printf( CTX.Temp, "[ComponentInfo]{ ID: %i, Name: %s, Used: %i/%i }",
		Value.ID, Value.Name, Value.GetManager()->CountUsed(), Value.GetManager()->CountTotal() );
}
