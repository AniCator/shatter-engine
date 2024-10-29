// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

// Fixed-size vector.
template<typename T>
struct FixedVector
{
	FixedVector( const size_t& Size )
	{
		this->Size = Size;
		Data = new T[Size];
	}

	~FixedVector()
	{
		delete Data;
	}

	T& operator[]( const size_t& Index )
	{
		return Data[Index];
	}

	const T& operator[]( const size_t& Index ) const
	{
		return Data[Index];
	}

	size_t size() const
	{
		return Size;
	}

	bool empty() const
	{
		return Size == 0;
	}

	// NOTE: Might not need this.
	struct Iterator
	{
		Iterator( T* Pointer ) : Pointer( Pointer ) {}

		bool operator!=( const Iterator& Other ) const
		{
			return Pointer != Other.Pointer;
		}

		const T& operator*() const
		{
			return *Pointer;
		}

		Iterator operator++()
		{
			++Pointer;
			return *this;
		}

		T* Pointer;
	};

	// begin() iterator used by ranged for-loops
	Iterator begin() const
	{
		return Iterator( Data );
	}

	// end() iterator used by ranged for-loops
	Iterator end() const
	{
		return Iterator( Data + Size );
	}

	T& front()
	{
		return Data[0];
	}

	const T& front() const
	{
		return Data[0];
	}

	T& back()
	{
		return Data[Size - 1];
	}

	const T& back() const
	{
		return Data[Size - 1];
	}

	void swap( const size_t A, const size_t B )
	{
		if( A >= Size || B >= Size )
			return; // Can't swap outside of the size range.

		std::swap( Data[A], Data[B] );
	}

	// Copy
	FixedVector( FixedVector const& Vector )
	{
		if( this == &Vector )
			return;

		delete Data;
		Size = Vector.Size;
		Data = new T[Size];

		for( size_t Index = 0; Index < Size; ++Index )
		{
			Data[Index] = Vector.Data[Index];
		}
	}

	FixedVector& operator=( FixedVector const& Vector )
	{
		if( this == &Vector )
			return *this;

		delete Data;
		Size = Vector.Size;
		Data = new T[Size];

		for( size_t Index = 0; Index < Size; ++Index )
		{
			Data[Index] = Vector.Data[Index];
		}

		return *this;
	}

	// Move.
	FixedVector( FixedVector&& Vector ) noexcept
	{
		Size = Vector.Size;
		Data = Vector.Data;

		Vector.Size = 0;
		Vector.Data = nullptr;
	}

	FixedVector& operator=( FixedVector&& Vector ) noexcept
	{
		if( this == &Vector )
			return *this;

		Size = Vector.Size;
		Data = Vector.Data;

		Vector.Size = 0;
		Vector.Data = nullptr;

		return *this;
	}

	FixedVector() = default;
protected:
	size_t Size = 0;
	T* Data = nullptr;
};

// Dodgy dynamic vector that allocates double the size when it runs out of room.
template<typename T>
struct GreedyVector
{
	GreedyVector( const size_t& Size )
	{
		this->Size = 0;
		this->Capacity = Size == 0 ? 1 : Size;
		Data = new T[Capacity];
	}

	~GreedyVector()
	{
		delete Data;
	}

	T& operator[]( const size_t& Index )
	{
		if( Index >= Size && Index < Capacity )
		{
			Size = Index + 1;
		}

		return Data[Index];
	}

	const T& operator[]( const size_t& Index ) const
	{
		return Data[Index];
	}

	size_t size() const
	{
		return Size;
	}

	size_t capacity() const
	{
		return Capacity;
	}

	bool empty() const
	{
		return Size == 0;
	}

	void clear()
	{
		Size = 0;
	}

	// NOTE: Might not need this.
	struct Iterator
	{
		Iterator( T* Pointer ) : Pointer( Pointer ) {}

		bool operator!=( const Iterator& Other ) const
		{
			return Pointer != Other.Pointer;
		}

		const T& operator*() const
		{
			return *Pointer;
		}

		Iterator operator++()
		{
			++Pointer;
			return *this;
		}

		T* Pointer;
	};

	// begin() iterator used by ranged for-loops
	Iterator begin() const
	{
		return Iterator( Data );
	}

	// end() iterator used by ranged for-loops
	Iterator end() const
	{
		return Iterator( Data + Size );
	}

	T& front()
	{
		return Data[0];
	}

	const T& front() const
	{
		return Data[0];
	}

	T& back()
	{
		return Data[Size - 1];
	}

	const T& back() const
	{
		return Data[Size - 1];
	}

	void add( const T& Item )
	{
		// Check if we're at capacity.
		if( Size == Capacity )
		{
			// Grow the vector to fit new data.
			grow();
		}

		Data[Size] = Item;
		Size++;
	}

	// Remove the item at the index, and close the gap. (slow, maintains ordering)
	void remove( const size_t Index )
	{
		T* New = new T[Capacity];

		// Copy the data from the old address.
		size_t NewLocation = 0;
		for( size_t Location = 0; Location < Size; ++Location )
		{
			if( Location == Index )
				continue; // Skip over the location we want to remove.

			New[NewLocation] = Data[Location];
			++NewLocation;
		}

		// Remove the data from the old location.
		delete Data;

		// Assign the new address.
		Data = New;

		// Reduce the size by one.
		Size -= 1;
	}

	void swap( const size_t A, const size_t B )
	{
		if( A >= Size || B >= Size )
			return; // Can't swap outside of the size range.

		std::swap( Data[A], Data[B] );
	}

	// Pop the tail.
	void pop()
	{
		if( empty() )
			return; // Already empty.
		
		Size -= 1;
	}

	// Swap the entry with the tail entry and pop it. (fast, changes ordering)
	void remove_unordered( const size_t Index )
	{
		if( Index >= Size )
			return; // Can't swap outside of the size range.

		swap( Index, Size - 1 );
		pop();
	}

	// Double the capacity.
	void grow()
	{
		reserve( Capacity * 2 );
	}

	void reserve( const size_t Capacity )
	{
		if( Capacity <= this->Capacity )
			return; // No need to reserve additional space.

		this->Capacity = Capacity;
		T* New = new T[Capacity];

		// Copy the data from the old address.
		for( size_t Location = 0; Location < Size; ++Location )
		{
			New[Location] = Data[Location];
		}

		// Remove the data from the old location.
		delete Data;

		// Assign the new address.
		Data = New;
	}

	// Copy
	GreedyVector( GreedyVector const& Vector )
	{
		if( this == &Vector )
			return;

		delete Data;
		Capacity = Vector.Capacity;
		Size = Vector.Size;
		Data = new T[Capacity];

		for( size_t Location = 0; Location < Size; ++Location )
		{
			Data[Location] = Vector.Data[Location];
		}
	}

	GreedyVector& operator=( GreedyVector const& Vector )
	{
		if( this == &Vector )
			return *this;

		delete Data;
		Capacity = Vector.Capacity;
		Size = Vector.Size;
		Data = new T[Capacity];

		for( size_t Location = 0; Location < Size; ++Location )
		{
			Data[Location] = Vector.Data[Location];
		}

		return *this;
	}

	// Move.
	GreedyVector( GreedyVector&& Vector ) noexcept
	{
		Capacity = Vector.Capacity;
		Size = Vector.Size;
		Data = Vector.Data;

		Vector.Capacity = 0;
		Vector.Size = 0;
		Vector.Data = nullptr;
	}

	GreedyVector& operator=( GreedyVector&& Vector ) noexcept
	{
		if( this == &Vector )
			return *this;

		Capacity = Vector.Capacity;
		Size = Vector.Size;
		Data = Vector.Data;

		Vector.Capacity = 0;
		Vector.Size = 0;
		Vector.Data = nullptr;

		return *this;
	}

	GreedyVector() = default;
protected:
	size_t Capacity = 0;
	size_t Size = 0;
	T* Data = nullptr;
};
