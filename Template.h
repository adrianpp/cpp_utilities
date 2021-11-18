#ifndef MY_TEMPLATE_H__
#define MY_TEMPLATE_H__

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <assert.h>
#include <algorithm>
#include <sstream>

template<class It>
It advance(It iterator, int count, It end)
{
	for(int i = 0; iterator != end and i < count; ++i)
		++iterator;
	return iterator;
}

template<class Key, class Value>
bool isIn(std::map<Key,Value>& m, const Key& k)
{
	return (m.find(k) != m.end());
}

template<class Key, class Value>
bool isIn(std::unordered_map<Key,Value>& m, const Key& k)
{
	return (m.find(k) != m.end());
}

template<class T>
bool isIn(const std::vector<T>& m, const T& k)
{
	return (std::find(m.begin(), m.end(), k) != m.end());
}


template<class T, class Value>
void erase(T& container, Value v)
{
	auto it = std::remove(std::begin(container), std::end(container), v);
	container.erase(it, container.end());
}
template<class T, class Pred>
void erase_if(T& container, Pred pred)
{
	auto it = std::remove_if(std::begin(container), std::end(container), pred);
	container.erase(it, container.end());
}

template<class T>
decltype(*std::declval<T>()) range_max(T b, T e)
{
	typedef decltype(*std::declval<T>()) RetTy;
	RetTy ret = *b;
	while( b != e ) {
		ret = (ret > *b) ? ret : *b;
		++b;
	}
	return ret;
}
	
template<class T>
decltype(*std::declval<T>()) range_min(T b, T e)
{
	typedef decltype(*std::declval<T>()) RetTy;
	RetTy ret = *b;
	while( b != e ) {
		ret = (ret < *b) ? ret : *b;
		++b;
	}
	return ret;
}

template<class T, class F>
bool test_pairs_any(T b, T e, F&& f)
{
	for(T b0 = b; b0 != e; ++b0)
		for(T b1 = b0+1; b1 != e; ++b1)
		{
			if( f(b0, b1) ) return true;
		}
	return false;
}

class Line {
	std::string s;
public:
	operator std::string() {return s;}
	friend std::istream& operator >> (std::istream& is, Line& line) {
		std::getline(is, line.s);
		return is;
	}
	friend std::ostream& operator << (std::ostream& os, const Line& line) {
		return os << line.s;
	}
};

uint64_t str_to_int(std::string s)
{
	std::stringstream ss(s);
	uint64_t ret;
	ss >> ret;
	return ret;
}

template<class Out, class In, class Func>
std::vector<Out> convert(std::vector<In>& input, Func f)
{
	std::vector<Out> ret;
	for(auto I : input)
	{
		ret.push_back(f(I));
	}
	return ret;
}

std::string getInputRaw()
{
	std::string input;
	while(std::cin.good() and !std::cin.eof())
	{
		std::string line;
		std::getline(std::cin,line);
		if( std::cin.eof() ) break;
		input += line + "\n";
	}
	return input;
}

std::vector<std::string> splitString(std::string input, std::string separator)
{
	std::vector<std::string> ret;
	size_t pos = 0;
	while( (pos = input.find(separator)) != std::string::npos )
	{
		std::string first = input.substr(0, pos);
		input.erase(0, pos + separator.length());
		ret.push_back(first);
	}
	if( input != "" )
		ret.push_back(input);
	return ret;
}

std::vector<std::string> splitString(std::vector<std::string> input, std::string separator)
{
	std::vector<std::string> ret;
	for(auto I : input)
	{
		std::vector<std::string> split = splitString(I, separator);
		for(auto S : split)
			if( S != "" )
				ret.push_back(S);
	}
	return ret;
}

std::vector<std::string> getInput(std::string separator="\n")
{
	return splitString(getInputRaw(), separator);
}

#endif /* MY_TEMPLATE_H__ */

