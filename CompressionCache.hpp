#ifndef _COMPRESSION_CACHE_HPP__
#define _COMPRESSION_CACHE_HPP__

#include <map>

template<class T, class IndexT = int>
class CompressionCacheValue {
private:
	static IndexT counter;
	static std::map<IndexT,T> cacheImpl;
	static std::map<T,IndexT> lookupImpl;
  static IndexT getIndex(T val)
	{
		//make sure val exists in cache
		typename std::map<T,IndexT>::iterator foundIter = lookupImpl.find(val);
    if( foundIter == lookupImpl.end() )
		{
			lookupImpl[val] = counter;
			cacheImpl[counter] = val;
			return counter++;
		}
		return foundIter->second;
	}
	//end statics
	IndexT index;
public:
	CompressionCacheValue() : index(getIndex(T())) {}
  CompressionCacheValue(T v) : index(getIndex(v)) {}
	operator T() {return cacheImpl[index];}
};

template<class T, class IndexT>
IndexT CompressionCacheValue<T,IndexT>::counter = 0;
template<class T, class IndexT>
std::map<IndexT,T> CompressionCacheValue<T,IndexT>::cacheImpl;
template<class T, class IndexT>
std::map<T,IndexT> CompressionCacheValue<T,IndexT>::lookupImpl;

#endif
