#ifndef CELLULAR_AUTOMATA_H__
#define CELLULAR_AUTOMATA_H__

#include <vector>
#include <array>

template<class T, std::size_t Dim>
struct multi_vector_impl {
	static_assert( Dim > 1 );
	using type = std::vector<typename multi_vector_impl<T,Dim-1>::type>;
};
template<class T>
struct multi_vector_impl<T,1> {
	using type = std::vector<T>;
};

template<class T, std::size_t Dim>
struct multi_vector_t : public multi_vector_impl<T,Dim>::type {
	typedef typename multi_vector_impl<T,Dim>::type type;
	using type::type;
	multi_vector_t(const type& v) : type(v) {}
	multi_vector_t<T,Dim-1>& child(int i)
	{
		typename multi_vector_t<T,Dim-1>::type& ret = type::at(i);
		return *static_cast<multi_vector_t<T,Dim-1>*>(&ret);
	}
};

template<class T, std::size_t Dim>
std::array<int,Dim> getDimensions(multi_vector_t<T,Dim>& vec)
{
	std::array<int,Dim> ret;
	ret.at(0) = vec.size();
	if constexpr (Dim-1 > 0)
	{
		std::array<int,Dim-1> rest = getDimensions(vec.child(0));
		for(int i = 0; i < Dim-1; ++i)
			ret.at(i+1) = rest.at(i);
	}
	return ret;
}

template<class T, std::size_t Dim, std::size_t Dim2>
auto getElement(multi_vector_t<T,Dim>& vec, std::array<int,Dim2> F)
{
	static_assert( Dim2 >= Dim );
	if constexpr (Dim-1 > 0)
		return getElement(vec.child(F.at(Dim2-Dim)), F);
	else
		return vec.at(F.at(Dim2-Dim));
}

template<int I, typename T, int Dim>
void recursive_resize(T& vec, std::array<int,Dim> sizes)
{
	vec.resize(sizes.at(I));
	if constexpr (I > 0)
		for(auto& V : vec)
			recursive_resize<I-1>(V, sizes);
}


#include "TemplateConfig.h"
#include <map>
#include <unordered_map>
#include <functional>

template <typename T, typename = std::void_t<>>
struct is_std_hashable : std::false_type { };

template <typename T>
struct is_std_hashable<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>> : std::true_type { };

template <typename T>
constexpr bool is_std_hashable_v = is_std_hashable<T>::value;


// should provide:
//   static constexpr void addNeighborCounts(std::unordered_map<T,int>& neighborCounts, const T& location);
// if the type T is hashable, and
//   static constexpr void addNeighborCounts(std::map<T,int>& neighborCounts, const T& location);
// if not.
template<class T>
struct CALocationHelper;

struct CAConfigDim {};
struct CAConfigCellState {};
struct CAConfigLocation {};
struct CAConfigAliveState {};

template<class... Args>
class CellularAutomata {
	static const int Dim = MyConfig::GetValueOrDefault<CAConfigDim, 2, Args...>::value;
	using CellState = typename MyConfig::GetTypeOrDefault<CAConfigCellState, bool, Args...>::type;
	using Location = typename MyConfig::GetTypeOrDefault<CAConfigLocation, std::array<int,Dim>, Args...>::type;
	static const CellState AliveState = MyConfig::GetValueOrDefault<CAConfigAliveState, true, Args...>::value;

	//use std::unordered_map if the type Value is hashable, otherwise use std::map
	template<class Value>
	using GridType = typename std::conditional<
			is_std_hashable_v<Location>,
			std::unordered_map<Location,Value>,
			std::map<Location,Value>
		>::type;

	GridType<CellState> cells;
	const std::function<CellState(CellState curState, int neighborCount)> updateFunc;
public:
	CellularAutomata(decltype(updateFunc) func) : updateFunc(func) {}
	CellState& getCell(Location loc)
	{
		return cells[loc];
	}
	int getNumberOfCellsOfState(CellState state)
	{
		int count = 0;
		for(auto C : cells)
			if( C.second == state )
				++count;
		return count;
	}
	void doOneStep()
	{
		//calculate neighbor counts
		GridType<int> neighborCount;
		for(auto C : cells)
		{
			if( C.second != AliveState ) continue;
			CALocationHelper<Location>::addNeighborCounts(neighborCount, C.first);
		}
		//update based on user-supplied function
		decltype(cells) next;
		for(auto N : neighborCount)
		{
			next[N.first] = updateFunc(cells[N.first], N.second);
		}
		cells = next;
	}
	multi_vector_t<CellState,Dim> locationMap()
	{
		multi_vector_t<CellState,Dim> ret;
		std::array<int,Dim> minDims, maxDims;
		minDims.fill(10000);
		maxDims.fill(-10000);
		for(auto C : cells)
		{
			if( C.second != AliveState ) continue;
			for(int d = 0; d < Dim; ++d)
			{
				minDims.at(d) = std::min(minDims.at(d), C.first.at(d));
				maxDims.at(d) = std::max(maxDims.at(d), C.first.at(d));
			}
		}
		std::array<int,Dim> len;
		for(int i = 0; i < Dim; ++i)
		{
			len.at(Dim-i-1) = maxDims.at(i) - minDims.at(i) + 1;
		}
		recursive_resize<Dim-1>(ret, len);
		for(auto C : cells)
		{
			if( C.second != AliveState ) continue;
			auto loc = C.first;
			for(int i = 0; i < Dim; ++i)
				loc.at(i) -= minDims.at(i);
			getElement(ret, loc) = C.second;
		}
		return ret;
	}
};

#include "MultiForVar.h"

template<std::size_t Dim>
struct CALocationHelper<std::array<int,Dim>> {
	typedef std::array<int,Dim> T;
	static constexpr void addNeighborCounts(std::unordered_map<T,int>& neighborCounts, const T& location)
	{
		std::array<int,Dim> start{}, end{};
		start.fill(-1);
		end.fill(1+1); // go from [-1,2)
		for(MultiForVar<Dim> F(start,end); !F.done(); ++F)
		{
			bool isSelf = true;
			for(int i = 0; i < Dim; ++i)
				if( F.index(i) != 0 )
					isSelf = false;
			if( isSelf ) continue;
			auto neighborLoc = location;
			for(int i = 0; i < Dim; ++i)
				neighborLoc[i] += F.index(i);
			neighborCounts[neighborLoc]++;
		}
	}
};

#endif /* CELLULAR_AUTOMATA_H__ */

