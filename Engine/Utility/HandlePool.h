// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

constexpr size_t InvalidPoolHandle = -1;

using PoolHandle = size_t;
using HandleMap = std::unordered_map<std::string, PoolHandle>;

/// <summary>
/// A pool that stores data and a map of strings, allowing the user to lookup data by their entry names or using their handles directly.
/// The latter being the faster method.
/// </summary>
/// <typeparam name="Type">The kind of data this pool will store.</typeparam>
template<typename Type>
struct HandlePool
{
	/// <summary>
	/// Adds an entry to the pool, if it doesn't exist yet.
	/// </summary>
	/// <param name="Name">The name we want to assign to the entry.</param>
	/// <param name="Entry">The entry itself.</param>
	/// <returns>True, if the entry was succesfully created. False, when the entry already exists under the given name.</returns>
	bool Create( const std::string& Name, Type Entry )
	{
		if( Exists( Name ) )
			return false; // Entry already exists.

		NameToHandle[Name] = Entries.size();
		Entries.emplace_back( Entry );

		return true;
	}

	/// <summary>
	/// Adds an entry to the pool; if it exists, the present entry is overwritten.
	/// </summary>
	/// <param name="Name">The name this entry will be associated with.</param>
	/// <param name="Entry">The data that will be created or assigned.</param>
	void CreateOrAssign( const std::string& Name, Type Entry )
	{
		const auto Handle = Get( Name );
		if( Handle != InvalidPoolHandle )
		{
			// The entry exists, overwrite it.
			Entries[Handle] = Entry;
			return;
		}

		// The entry doesn't exist yet, create a new one.
		NameToHandle[Name] = Entries.size();
		Entries.emplace_back( Entry );
	}

	/// <summary>
	/// Looks up an entry by its name and returns true if it is valid.
	/// </summary>
	/// <param name="Name">The entry name we want to search for.</param>
	/// <returns>Whether or not it is valid.</returns>
	bool Exists( const std::string& Name ) const
	{
		const auto Iterator = NameToHandle.find( Name );
		if( Iterator == NameToHandle.end() )
			return false;

		return true;
	}

	/// <summary>
	/// Looks up an entry by its handle and returns it.
	/// </summary>
	/// <param name="Handle">The handle of the entry we'd like to fetch.</param>
	/// <returns>The entry.</returns>
	///	<remarks>Cheapest function for pool access.</remarks>
	Type& Get( const PoolHandle& Handle ) const
	{
		if( Handle >= Entries.size() || Handle == InvalidPoolHandle )
			return {}; // Invalid handle.

		return Entries[Handle];
	}

	/// <summary>
	/// Looks up the name of an entry and returns its handle.
	/// </summary>
	/// <param name="Name">The entry name we want to search for.</param>
	/// <returns>The handle of the entry, returns InvalidPoolHandle if not found.</returns>'
	///	<remarks>Pass in a PoolHandle when you can, string lookups are more expensive.</remarks>
	PoolHandle Get( const std::string& Name ) const
	{
		const auto Iterator = NameToHandle.find( Name );
		if( Iterator == NameToHandle.end() )
			return InvalidPoolHandle;

		return Iterator->second;
	}

	/// <returns>The map of entry names and handles.</returns>
	const HandleMap& Get() const
	{
		return NameToHandle;
	}

	/// <returns>A vector of entry pointers.</returns>
	const std::vector<Type>& GetData() const
	{
		return Entries;
	}

	/// <param name="Original">The entry name we're looking for.</param>
	/// <param name="Name">The new name we want to assign to the entry.</param>
	/// <returns>False, if the entry name could not be found.</returns>
	bool Rename( const std::string& Original, const std::string& Name )
	{
		const auto Iterator = NameToHandle.find( Original );
		if( Iterator == NameToHandle.end() )
			return false; // The original name could not be found.

		// Cache the handle.
		const auto Handle = Iterator->second;

		// Delete the original name.
		NameToHandle.erase( Iterator );

		// Assign the new name.
		NameToHandle[Name] = Handle;

		return true;
	}

protected:
	HandleMap NameToHandle;
	std::vector<Type> Entries;
};