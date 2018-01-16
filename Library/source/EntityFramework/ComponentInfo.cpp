#include "EntityFramework/ComponentInfo.h"

std::ostream& operator<<( std::ostream& Stream, const ComponentInfo& Info )
{
	Stream << "[ComponentInfo]{ ID: " << Info.GetID() << ", Name: " << Info.GetName();
	Stream << ", Used: " << Info.GetManager()->CountUsed() << "/" << Info.GetManager()->CountTotal() << " }";
	return Stream;
}

DESCRIPTION( ComponentInfo )
{
	return l_printf( CTX, "[ComponentInfo]{ ID: %i, Name: %s, Used: %i/%i }",
		Value.ID, Value.Name, Value.GetManager()->CountUsed(), Value.GetManager()->CountTotal() );
}
