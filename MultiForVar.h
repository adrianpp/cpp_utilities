#ifndef MULTI_FOR_VAR_H__
#define MULTI_FOR_VAR_H__

#include <array>
#include <functional>

template<std::size_t Dim>
class MultiForVar {
	std::array<int,Dim> ind;
	const std::array<int,Dim> start, end;
	std::function<void(int i)> loopTrigger;

	template<int I>
		inline void increment_helper()
		{
			static_assert(I < Dim);
			static_assert(I >= 0);
			++index<I>();
			if( index<I>() >= end.at(I) )
			{
				if( loopTrigger and !done() ) loopTrigger(I);
				if constexpr ( I-1 >= 0 )
				{
					index<I>() = start.at(I);
					increment_helper<I-1>();
				}
			}
		}
	public:
	MultiForVar(
			const std::array<int,Dim>& start,
			const std::array<int,Dim>& end) : ind{start}, start(start), end(end) {}
	MultiForVar(
			const std::array<int,Dim>& start,
			const std::array<int,Dim>& end,
			decltype(loopTrigger) loopTrigger) : ind{start}, start(start), end(end), loopTrigger(loopTrigger) {}
	bool done() {return index<0>() >= end.at(0);}
	inline MultiForVar& operator++(){
		increment_helper<Dim-1>(); //Dim-1 is least significant index
		return *this;
	}
	template<int I>
		constexpr int& index()
		{
			static_assert(I < Dim);
			return ind[I];
		}
	constexpr int& index(int I)
	{
		return ind.at(I);
	}
	std::array<int,Dim> getIndexes(){return ind;}
};

#endif /* MULTI_FOR_VAR_H__ */

