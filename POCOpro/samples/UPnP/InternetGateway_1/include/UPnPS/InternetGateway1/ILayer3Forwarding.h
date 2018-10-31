//
// ILayer3Forwarding.h
//
// Library: InternetGateway1
// Package: Generated
// Module:  ILayer3Forwarding
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


#ifndef InternetGateway1_ILayer3Forwarding_INCLUDED
#define InternetGateway1_ILayer3Forwarding_INCLUDED


#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/RemotingNG/Identifiable.h"
#include "Poco/RemotingNG/Listener.h"
#include "UPnPS/InternetGateway1/InternetGateway1.h"
#include "UPnPS/InternetGateway1/Layer3Forwarding.h"


namespace UPnPS {
namespace InternetGateway1 {


class InternetGateway1_API ILayer3Forwarding: public virtual Poco::RefCountedObject
{
public:
	typedef Poco::AutoPtr<ILayer3Forwarding> Ptr;

	ILayer3Forwarding();
		/// Creates a ILayer3Forwarding.

	virtual ~ILayer3Forwarding();
		/// Destroys the ILayer3Forwarding.

	virtual void getDefaultConnectionService(std::string& newDefaultConnectionService) = 0;

	virtual std::string remoting__enableEvents(Poco::RemotingNG::Listener::Ptr pListener, bool enable = bool(true)) = 0;
		/// Enable or disable delivery of remote events.
		///
		/// The given Listener instance must implement the Poco::RemotingNG::EventListener
		/// interface, otherwise this method will fail with a RemotingException.
		///
		/// This method is only used with Proxy objects; calling this method on a
		/// RemoteObject will do nothing.

	static const Poco::RemotingNG::Identifiable::TypeId& remoting__typeId();
		/// Returns the TypeId of the class.

	virtual void setDefaultConnectionService(const std::string& newDefaultConnectionService) = 0;

	Poco::BasicEvent < const std::string > defaultConnectionServiceChanged;
};


} // namespace InternetGateway1
} // namespace UPnPS


#endif // InternetGateway1_ILayer3Forwarding_INCLUDED

