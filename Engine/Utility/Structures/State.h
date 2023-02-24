#include <cstdint>

#include <Game/Game.h>

template<class T, T Default>
struct State
{
	static std::underlying_type_t<T> Convert( const T Value )
	{
		return static_cast<std::underlying_type_t<T>>( Value );
	}

	static T Convert( const std::underlying_type_t<T> Value )
	{
		return static_cast<T>( Value );
	}

	T Get() const
	{
		return Current;
	}

	std::underlying_type_t<T> GetUnderlying() const
	{
		return Convert( Current );
	}

	T GetPrevious() const
	{
		return Previous;
	}

	std::underlying_type_t<T> PreviousUnderlying() const
	{
		return Convert( Previous );
	}

	bool Set( T Target )
	{
		if( Current == Target )
			return false; // We're already in this state.

		Previous = Current;
		Current = Target;

		LastTransitionTime = GameLayersInstance->GetCurrentTime();

		return true;
	}

	bool Set( std::underlying_type_t<T> Underlying )
	{
		Set( Convert( Underlying ) );
	}

	double TimeSinceTransition() const
	{
		return GameLayersInstance->GetCurrentTime() - LastTransitionTime;
	}

private:
	T Current = Default;
	T Previous = Default;

	double LastTransitionTime = 0.0;
};