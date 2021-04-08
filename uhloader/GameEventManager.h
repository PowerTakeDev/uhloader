#pragma once
#include <tier1/KeyValues.h>
#include <tier1/utlvector.h>
#include <igameevents.h>

class CGameEventCallback
{
public:
    void* m_pCallback;		// callback pointer
    int					m_nListenerType;	// client or server side ?
};

struct CGameEventDescriptor
{
public:
    CGameEventDescriptor()
    {
        name[0] = 0;
        eventid = -1;
        keys = NULL;
        local = false;
        reliable = true;
    }

public:
    char		name[MAX_EVENT_NAME_LENGTH];	// name of this event
    int			eventid;	// network index number, -1 = not networked
    KeyValues* keys;		// KeyValue describing data types, if NULL only name 
    bool		local;		// local event, never tell clients about that
    bool		reliable;	// send this event as reliable message
    CUtlVector<CGameEventCallback*>	listeners;	// registered listeners
};


class CGameEventManager
{
public:
    enum
    {
        SERVERSIDE = 0,		// this is a server side listener, event logger etc
        CLIENTSIDE,			// this is a client side listenet, HUD element etc
        CLIENTSTUB,			// this is a serverside stub for a remote client listener (used by engine only)
        SERVERSIDE_OLD,		// legacy support for old server event listeners
        CLIENTSIDE_OLD,		// legecy support for old client event listeners
    };
};