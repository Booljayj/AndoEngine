#include "EntityFramework/ComponentInfo.h"

ostream& operator<<( ostream& Stream, const ComponentInfo& Info )
{
	Stream << "[ComponentInfo]{ ID: " << Info.GetID() << ", Name: " << Info.GetName();
	Stream << ", Used: " << Info.GetManager()->CountUsed() << "/" << Info.GetManager()->CountTotal() << " }\n";
	return Stream;
}
