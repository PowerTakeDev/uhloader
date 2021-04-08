#pragma once
#include "protocol.h"

#include <inetmessage.h>
#include <inetchannel.h>
#include <inetmsghandler.h>
#include <bitvec.h>
#include <igameevents.h>
#include <bitbuf.h>

#define DECLARE_BASE_MESSAGE( msgtype )						\
	public:													\
		bool			ReadFromBuffer( bf_read &buffer );	\
		bool			WriteToBuffer( bf_write &buffer );	\
		const char		*ToString() const;					\
		int				GetType() const { return msgtype; } \
		const char		*GetName() const { return #msgtype;}\

#define DECLARE_NET_MESSAGE( name )			\
	DECLARE_BASE_MESSAGE( net_##name );		\
	INetMessageHandler *m_pMessageHandler;	\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

#define DECLARE_SVC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( svc_##name );	\
	IServerMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

#define DECLARE_CLC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( clc_##name );	\
	IClientMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

#define DECLARE_MM_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( mm_##name );	\
	IMatchmakingMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

class CNetMessage : public INetMessage
{
public:
	CNetMessage() {
		m_bReliable = true;
		m_NetChannel = NULL;
	}

	virtual ~CNetMessage() {};

	virtual int		GetGroup() const { return INetChannelInfo::GENERIC; }
	INetChannel* GetNetChannel() const { return m_NetChannel; }

	virtual void	SetReliable(bool state) { m_bReliable = state; };
	virtual bool	IsReliable() const { return m_bReliable; };
	virtual void    SetNetChannel(INetChannel* netchan) { m_NetChannel = netchan; }
	virtual bool	Process() { return false; };	// no handler set

protected:
	bool				m_bReliable;	// true if message should be send reliable
	INetChannel* m_NetChannel;	// netchannel this message is from/for
};

class CLC_ListenEvents : public CNetMessage
{
	DECLARE_CLC_MESSAGE(ListenEvents);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

public:
	CBitVec<MAX_EVENT_NUMBER> m_EventArray;
};