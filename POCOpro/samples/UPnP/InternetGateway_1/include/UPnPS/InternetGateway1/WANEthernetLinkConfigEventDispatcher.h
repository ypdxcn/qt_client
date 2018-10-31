//
// WANEthernetLinkConfigEventDispatcher.h
//
// Library: InternetGateway1
// Package: Generated
// Module:  WANEthernetLinkConfigEventDispatcher
//
// This file has been generated.
// Warning: All changes to this will be lost when the file is re-generated.
//
// Copyright (c) 2011-2016, Applied Informatics Software Engineering GmbH.
// All rights reserved.
// 
// This is unpublished proprietary source code of Applied Informatics.
// The contents of this file may not be disclosed to third parties,
// copied or duplicated in any form, in whole or in part.
// 
//


#ifndef InternetGateway1_WANEthernetLinkConfigEventDispatcher_INCLUDED
#define InternetGateway1_WANEthernetLinkConfigEventDispatcher_INCLUDED


#include "Poco/RemotingNG/EventDispatcher.h"
#include "UPnPS/InternetGateway1/InternetGateway1.h"
#include "UPnPS/InternetGateway1/WANEthernetLinkConfigRemoteObject.h"


namespace UPnPS {
namespace InternetGateway1 {


class InternetGateway1_API WANEthernetLinkConfigEventDispatcher: public Poco::RemotingNG::EventDispatcher
{
public:
	WANEthernetLinkConfigEventDispatcher(WANEthernetLinkConfigRemoteObject* pRemoteObject, const std::string& protocol);
		/// Creates a WANEthernetLinkConfigEventDispatcher.

	virtual ~WANEthernetLinkConfigEventDispatcher();
		/// Destroys the WANEthernetLinkConfigEventDispatcher.

	void event__ethernetLinkStatusChanged(const void* pSender, const std::string& data);

	virtual const Poco::RemotingNG::Identifiable::TypeId& remoting__typeId() const;

private:
	void event__ethernetLinkStatusChangedImpl(const std::string& subscriberURI, const std::string& data);

	static const std::string DEFAULT_NS;
	WANEthernetLinkConfigRemoteObject* _pRemoteObject;
};


inline const Poco::RemotingNG::Identifiable::TypeId& WANEthernetLinkConfigEventDispatcher::remoting__typeId() const
{
	return IWANEthernetLinkConfig::remoting__typeId();
}


} // namespace InternetGateway1
} // namespace UPnPS


#endif // InternetGateway1_WANEthernetLinkConfigEventDispatcher_INCLUDED
