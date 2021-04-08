#include "netmessages.h"
#include <tier1/strtools.h>
#include "net.h"

bool CLC_ListenEvents::WriteToBuffer(bf_write& buffer)
{
	return true;
}

bool CLC_ListenEvents::ReadFromBuffer(bf_read& buffer)
{
	return true;
}

const char* CLC_ListenEvents::ToString(void) const
{
	return "";
}

