#include "EntityFramework/ComponentInfo.h"

DESCRIPTION( ComponentInfo )
{
	return l_printf( CTX, "[ComponentInfo]{ ID: %i, Name: %s, Used: %i/%i }",
		Value.ID, Value.Name, Value.GetManager()->CountUsed(), Value.GetManager()->CountTotal() );
}
