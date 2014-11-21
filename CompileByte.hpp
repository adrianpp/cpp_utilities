#if !defined(__GXX_EXPERIMENTAL_CXX0X__) && __cplusplus < 201103L
#error This file requires --std=c++0x in order to access static_cast!
#else

#include <iostream>
#include <stdint.h>
#include <assert.h>

template<class T,class BYTE=uint8_t>
class CompileByte {
public:
	static const unsigned numBytes = sizeof(T) / sizeof(BYTE);
private:
  union {
		T value;
		BYTE bytes[numBytes];
	} data;
public:
  CompileByte(T v) {data.value = v;}
	template<unsigned I> const BYTE getByte() const
	{
		static_assert(I<numBytes, "Too large of byte index specified!");
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return data.bytes[I];
#else
		return data.bytes[numBytes-I];
#endif
	}
	const BYTE getByte(unsigned I) const
	{
		assert(I<numBytes and "Too large of byte index specified!");
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return data.bytes[I];
#else
		return data.bytes[numBytes-I];
#endif
	}
	const unsigned getNumBytes() const {return numBytes;}
};

template<int S, class T, class B> struct printBytes_helper {
	static void apply(CompileByte<T,B> cb)
  {
	  printBytes_helper<S-1,T,B>::apply(cb);
	  std::cout << " " << cb.template getByte<S>();
  }
};

template<class T, class B> struct printBytes_helper<0,T,B> {
	static void apply(CompileByte<T,B> cb)
  {
	  std::cout << cb.template getByte<0>();
  }
};

template<class T, class B> void printBytes(CompileByte<T,B> cb)
{
  printBytes_helper<CompileByte<T,B>::numBytes-1,T,B>::apply(cb);
	std::cout << std::endl;
}

int main()
{
	CompileByte<int> cb(0x01020304);
	std::cout << cb.numBytes << std::endl;
	printBytes<>(cb);
	return -1;
}

#endif
