//
// AVTransport1Skeleton.h
//
// Library: MediaServer1
// Package: Generated
// Module:  AVTransport1Skeleton
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


#ifndef MediaServer1_AVTransport1Skeleton_INCLUDED
#define MediaServer1_AVTransport1Skeleton_INCLUDED


#include "Poco/RemotingNG/Skeleton.h"
#include "UPnPS/MediaServer1/AVTransport1RemoteObject.h"
#include "UPnPS/MediaServer1/MediaServer1.h"


namespace UPnPS {
namespace MediaServer1 {


class MediaServer1_API AVTransport1Skeleton: public Poco::RemotingNG::Skeleton
{
public:
	AVTransport1Skeleton();
		/// Creates a AVTransport1Skeleton.

	virtual ~AVTransport1Skeleton();
		/// Destroys a AVTransport1Skeleton.

	virtual const Poco::RemotingNG::Identifiable::TypeId& remoting__typeId() const;

	static const std::string DEFAULT_NS;
};


inline const Poco::RemotingNG::Identifiable::TypeId& AVTransport1Skeleton::remoting__typeId() const
{
	return IAVTransport1::remoting__typeId();
}


} // namespace MediaServer1
} // namespace UPnPS


#endif // MediaServer1_AVTransport1Skeleton_INCLUDED
