// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

constexpr size_t InvalidAssetHandle = -1;

using AssetHandle = size_t;
using AssetHandleMap = std::unordered_map<std::string, AssetHandle>;

struct AssetEntry
{
	AssetEntry() = default;
	AssetEntry( const AssetHandle& Handle )
	{
		this->Handle = Handle;
	}

	AssetHandle Handle = InvalidAssetHandle;
};

/// <summary>
/// A pool that stores pointers to assets.
/// </summary>
/// <typeparam name="AssetType">The asset type this pool will house.</typeparam>
template<typename AssetType>
struct AssetPool
{
	using AssetVector = std::vector<AssetType>;

	template<typename T>
	using Pointer = typename std::enable_if_t<std::is_pointer_v<T>, T>;
	template<typename T>
	using Reference = typename std::enable_if_t<!std::is_pointer_v<T>, T>;

	/// <summary>
	/// Adds an asset to the pool, if it doesn't exist yet.
	/// </summary>
	/// <param name="Name">The name we want to assign to the asset.</param>
	/// <param name="Asset">The pointer to the asset itself.</param>
	/// <returns>True, if the asset was succesfully created. False, when the asset already exists under the given name.</returns>
	bool Create( const std::string& Name, const AssetType& Asset )
	{
		if( Exists( Name ) )
			return false; // Asset already exists.

		NameToHandle[Name] = Assets.size();
		Assets.emplace_back( Asset );

		return true;
	}

	/// <summary>
	/// Looks up an asset by its name and returns the asset pointer if it is valid.
	/// </summary>
	/// <param name="Name">The asset name we want to search for.</param>
	/// <returns>The asset pointer, or null.</returns>
	template<typename T = AssetType>
	const Pointer<T>& Find( const std::string& Name ) const
	{
		const auto Iterator = NameToHandle.find( Name );
		if( Iterator == NameToHandle.end() )
		{
			static Pointer<T> Null = nullptr;
			return Null;
		}

		return Assets[Iterator->second];
	}

	/// <summary>
	/// Looks up an asset by its name and returns the asset reference.
	/// </summary>
	/// <param name="Name">The asset name we want to search for.</param>
	/// <returns>The asset reference, or throws an exception if the handle is invalid.</returns>
	template<typename T = AssetType>
	const Reference<T>& Find( const std::string& Name ) const
	{
		const auto Iterator = NameToHandle.find( Name );
		if( Iterator == NameToHandle.end() )
		{
			throw std::exception( "Invalid asset." );
		}

		return Assets[Iterator->second];
	}

	/// <summary>
	/// Looks up an asset by its name and returns the asset pointer if it is valid.
	/// </summary>
	/// <param name="Name">The asset name we want to search for.</param>
	/// <returns>The asset pointer, or null.</returns>
	template<typename T = AssetType>
	Pointer<T>& Find( const std::string& Name )
	{
		const auto Iterator = NameToHandle.find( Name );
		if( Iterator == NameToHandle.end() )
		{
			static Pointer<T> Null = nullptr;
			return Null;
		}

		return Assets[Iterator->second];
	}

	/// <summary>
	/// Looks up an asset by its name and returns the asset reference.
	/// </summary>
	/// <param name="Name">The asset name we want to search for.</param>
	/// <returns>The asset reference, or throws an exception if the handle is invalid.</returns>
	template<typename T = AssetType>
	Reference<T>& Find( const std::string& Name )
	{
		const auto Iterator = NameToHandle.find( Name );
		if( Iterator == NameToHandle.end() )
		{
			throw std::exception( "Invalid asset." );
		}

		return Assets[Iterator->second];
	}

	/// <summary>
	/// Looks up an asset by its handle and returns it.
	/// </summary>
	/// <param name="Handle">The handle of the asset we'd like to fetch.</param>
	/// <returns>The asset pointer, or null.</returns>
	template<typename T = AssetType>
	const Pointer<T>& Get( const AssetHandle& Handle ) const
	{
		if( Handle >= Assets.size() || Handle == InvalidAssetHandle )
		{
			static Pointer<T> Null = nullptr;
			return Null;
		}

		return Assets[Handle];
	}

	/// <summary>
	/// Looks up an asset by its handle and returns it.
	/// </summary>
	/// <param name="Handle">The handle of the asset we'd like to fetch.</param>
	/// <returns>The asset reference, or throws an exception if the handle is invalid.</returns>
	template<typename T = AssetType>
	const Reference<T>& Get( const AssetHandle& Handle ) const
	{
		if( Handle >= Assets.size() || Handle == InvalidAssetHandle )
		{
			throw std::exception( "Invalid asset." );
		}

		return Assets[Handle];
	}

	/// <summary>
	/// Looks up an asset by its handle and returns it.
	/// </summary>
	/// <param name="Handle">The handle of the asset we'd like to fetch.</param>
	/// <returns>The asset pointer, or null.</returns>
	template<typename T = AssetType>
	Pointer<T>& Get( const AssetHandle& Handle )
	{
		if( Handle >= Assets.size() || Handle == InvalidAssetHandle )
		{
			static Pointer<T> Null = nullptr;
			return Null;
		}

		return Assets[Handle];
	}

	/// <summary>
	/// Looks up an asset by its handle and returns it.
	/// </summary>
	/// <param name="Handle">The handle of the asset we'd like to fetch.</param>
	/// <returns>The asset reference, or throws an exception if the handle is invalid.</returns>
	template<typename T = AssetType>
	Reference<T>& Get( const AssetHandle& Handle )
	{
		if( Handle >= Assets.size() || Handle == InvalidAssetHandle )
		{
			throw std::exception( "Invalid asset." );
		}

		return Assets[Handle];
	}

	/// <summary>
	/// Looks up the name of an asset and returns its handle.
	/// </summary>
	/// <param name="Name">The asset name we want to search for.</param>
	/// <returns>The handle of the asset, returns InvalidAssetHandle if not found.</returns>
	AssetHandle Get( const std::string& Name ) const
	{
		const auto Iterator = NameToHandle.find( Name );
		if( Iterator == NameToHandle.end() )
			return InvalidAssetHandle;

		return Iterator->second;
	}

	/// <summary>
	/// Checks if a valid asset handle exists for the given name.
	/// </summary>
	/// <param name="Name">The name of the asset we're looking for.</param>
	/// <returns>Returns whether or not the asset exists.</returns>
	bool Exists( const std::string& Name ) const
	{
		return Get( Name ) != InvalidAssetHandle;
	}

	/// <returns>The map of asset names and handles.</returns>
	const AssetHandleMap& Get() const
	{
		return NameToHandle;
	}

	/// <returns>A vector of asset pointers.</returns>
	const AssetVector& GetAssets() const
	{
		return Assets;
	}

	/// <param name="Original">The asset name we're looking for.</param>
	/// <param name="Name">The new name we want to assign to the asset.</param>
	/// <returns>False, if the asset name could not be found.</returns>
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
	AssetHandleMap NameToHandle;
	AssetVector Assets;
};