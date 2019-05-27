// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class IEngineService;

template<class ServiceType, class NullType>
class TLocator
{
public:
	static void Initialize()
	{
		Service = static_cast<IEngineService*>( &NullService );
	}

	static void Assign( IEngineService* ServiceIn )
	{
		if( ServiceIn )
		{
			Service = ServiceIn;
		}
		else
		{
			Service = static_cast<IEngineService*>( &NullService );
		}
	}

	static ServiceType& Get()
	{
		if( !Service )
		{
			Service = static_cast<IEngineService*>( &NullService );
		}

		return *static_cast<ServiceType*>( Service );
	}

private:
	static IEngineService* Service;
	static NullType NullService;
};

template<class ServiceType, class NullType>
IEngineService* TLocator<ServiceType, NullType>::Service = nullptr;

template<class ServiceType, class NullType>
NullType TLocator<ServiceType, NullType>::NullService = NullType();
