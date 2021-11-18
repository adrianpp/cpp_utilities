#ifndef WINDOW_ITERATOR_H__
#define WINDOW_ITERATOR_H__

#include <vector>
#include <array>
#include <string>

/*
   MULTIFORVAR
*/

template<int Dim>
class MultiForVar {
	std::array<int,Dim> ind;
	const std::array<int,Dim> start, end;

	template<int I>
	inline void increment_helper()
	{
		static_assert(I < Dim);
		const int ind = Dim-I-1;
		++index<ind>();
		if( index<ind>() > end.at(I) )
		{
			if constexpr ( I+1 < Dim )
			{
				index<ind>() = start.at(I);
				increment_helper<I+1>();
			}
		}
	}
public:	
	MultiForVar(
		const std::array<int,Dim>& start,
		const std::array<int,Dim>& end) : ind{start}, start(start), end(end) {}
	bool done() {return index<0>() > end.at(0);}
	inline MultiForVar& operator++() {
		increment_helper<0>();
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
};

/*
   WINDOW ITERATOR
*/

template<class T> struct WindowIteratorHelper;

template<class T>
struct WindowIteratorHelper<std::vector<T>> {
	static T& GetElementAtIndex(std::vector<T>& t, int i) {return t.at(i);}
	static int GetMaxSize(std::vector<T>& t){return t.size();}
};

template<class T, std::size_t N>
struct WindowIteratorHelper<std::array<T,N>> {
	static T& GetElementAtIndex(std::array<T,N>& t, int i) {return t.at(i);}
	static int GetMaxSize(std::array<T,N>& t){return N;}
};

template<>
struct WindowIteratorHelper<std::string> {
	static auto& GetElementAtIndex(std::string& t, int i) {return t.at(i);}
	static int GetMaxSize(std::string& t){return t.size();}
};

template<class T, int Dim>
class WindowIterator {
	T& data;
	std::array<int,Dim> ind;
	const std::array<int,Dim> win_size;

	template<int I, typename A, typename...RestIndexes>
	inline auto& get_partial(A& a, int i, RestIndexes...rest)
	{
		static_assert( sizeof...(rest)+1 == Dim-I );
		if constexpr (sizeof...(rest) == 0)
			return  WindowIteratorHelper<A>::GetElementAtIndex(a,index<I>()+i);
		else
			return get_partial<I+1>(WindowIteratorHelper<A>::GetElementAtIndex(a,index<I>()+i), rest...);
	}
	template<int I, class A>
	constexpr int max(A& a)
	{
		static_assert(I < Dim);
		static_assert( I >= 0 );
		if constexpr ( I == 0 )
			return WindowIteratorHelper<A>::GetMaxSize(a);
		else
			return max<I-1>(WindowIteratorHelper<A>::GetElementAtIndex(a,0));
	}
	template<int I>
	inline void increment_helper()
	{
		static_assert(I < Dim);
		const int ind = Dim-I-1;
		++index<ind>();
		if( index<ind>() + win_size.at(ind) > max<ind>(data) )
		{
			if constexpr ( I+1 < Dim )
			{
				index<ind>() = 0;
				increment_helper<I+1>();
			}
		}
	}
	template<class Other>
	WindowIterator(Other& data, WindowIterator<Other,Dim> other) : 
			data(data), ind{other.ind}, win_size(other.win_size) {}
public:
	template<typename...Dims>
	WindowIterator(T& data, Dims...d) : data(data), ind{0}, win_size({d...}) {
		static_assert(sizeof...(d) == Dim);
	}
	bool done() {return index<0>() + win_size.at(0) > max<0>(data);}
	inline WindowIterator& operator++(){
		increment_helper<0>();
		return *this;
	}
	template<int I>
	constexpr int& index()
	{
		static_assert(I < Dim);
		return ind[I];
	}
	template<typename...Rest>
	inline auto& get(int i, Rest...rest)
	{
		static_assert(sizeof...(rest)+1 <= Dim);
		return get_partial<0>(data, i, rest...);
	}
	template<class Other>
	WindowIterator<Other,Dim> on(Other& other_array)
	{
		return WindowIterator<Other,Dim>(other_array, *this);
	}
	template<int I>
	constexpr int getWinSize()
	{
		static_assert(I < Dim);
		return win_size[I];
	}
	template<typename...Rest>
	inline void setIndexes(int i, Rest...rest)
	{
	}
};

template<class T, typename...Dims>
WindowIterator(T, Dims...) -> WindowIterator<T, sizeof...(Dims)>;

/* =============================
   multi_array_t / multi_vector_t
   ============================= */

namespace details {

template<class T, int I, int...Rest>
struct multi_array_impl {
	using type = std::array<typename multi_array_impl<T,Rest...>::type, I>;
};
template<class T, int I>
struct multi_array_impl<T,I> {
	using type = std::array<T, I>;
};

template<class T, int Dim>
struct multi_vector_impl {
	static_assert( Dim > 1 );
	using type = std::vector<typename multi_vector_impl<T,Dim-1>::type>;
};
template<class T>
struct multi_vector_impl<T,1> {
	using type = std::vector<T>;
};

} /* namespace details */

template<class T,int I, int...Rest>
class multi_array_t : public details::multi_array_impl<T, I, Rest...>::type {
	template<class A>
	inline constexpr auto& atHelper(A& a, int FirstIndex)
	{
		return a.at(FirstIndex);
	}
	template<class A, class...RestIndexTy>
	inline constexpr auto& atHelper(A& a, int FirstIndex, RestIndexTy...RestIndex)
	{
		return atHelper(a.at(FirstIndex), RestIndex...);
	}
public:
	typedef typename details::multi_array_impl<T, I, Rest...>::type type;
	using type::type;
	template<class...RestIndexTy>
	inline constexpr auto& at(int FirstIndex, RestIndexTy...RestIndex)
	{
		return atHelper(*static_cast<type*>(this),FirstIndex,RestIndex...);
	}
	void fill(T value)
	{
		if constexpr (sizeof...(Rest))
		{
			multi_array_t<T,Rest...> partial;
			partial.fill(value);
			type::fill(partial);
		}
		else
			type::fill(value);
	}
};

template<class T,int Dim>
class multi_vector_t : public details::multi_vector_impl<T, Dim>::type {
	template<class A>
	inline constexpr auto& atHelper(A& a, int FirstIndex)
	{
		return a.at(FirstIndex);
	}
	template<class A, class...RestIndexTy>
	inline constexpr auto& atHelper(A& a, int FirstIndex, RestIndexTy...RestIndex)
	{
		return atHelper(a.at(FirstIndex), RestIndex...);
	}
	template<class A, class...RestSizeTy>
	void resizeHelper(A& a, int firstSize, RestSizeTy...restSize)
	{
		a.resize(firstSize);
		if constexpr (sizeof...(restSize))
			for(auto& V : a)
				resizeHelper(V, restSize...);
	}
public:
	typedef typename details::multi_vector_impl<T, Dim>::type type;
	using type::type;
	template<class...RestIndexTy>
	inline constexpr auto& at(int FirstIndex, RestIndexTy...RestIndex)
	{
		return atHelper(*static_cast<type*>(this),FirstIndex,RestIndex...);
	}
	template<class...RestSizeTy>
	typename std::enable_if<(Dim > 1)>::type resize(int firstSize, RestSizeTy...restSize)
	{
		resizeHelper(*static_cast<type*>(this), firstSize, restSize...);
	}
};

template<class T, int Dim>
struct WindowIteratorHelper<multi_vector_t<T,Dim>> : public WindowIteratorHelper<typename multi_vector_t<T,Dim>::type> {};

template<class T, int I, int...Rest>
struct WindowIteratorHelper<multi_array_t<T,I,Rest...>> : public WindowIteratorHelper<typename multi_array_t<T,I,Rest...>::type> {};


#endif /* WINDOW_ITERATOR_H__ */


