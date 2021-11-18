#ifndef MY_REGEX_H__
#define MY_REGEX_H__

#include <string>
#include <sstream>
#include <regex>
#include <cassert>
#include <type_traits>

#if !defined(PRINT_MATCHES)
#define PRINT_MATCHES 0
#endif

#if PRINT_MATCHES
#include <iostream>
#endif

namespace MyRegex {

class RegexCache {
	std::map<std::string, std::regex> cache;
public:
	static RegexCache& instance() {
		static RegexCache inst;
		return inst;
	}
	std::regex get(std::string s)
	{
		if( cache.find(s) == cache.end() )
			cache.emplace(std::make_pair(s, std::regex(s)));
		assert( cache.find(s) != cache.end() );
		return cache.find(s)->second;
	}
	bool match(std::string s, std::string reg)
	{
		return std::regex_match(s, get(reg));
	}
	bool match(std::string s, std::smatch& sm, std::string reg)
	{
		return std::regex_match(s, sm, get(reg));
	}

};

/*
   A regex tree is composed of nodes, each node represents a regex pattern and
    possibly contains the values that the node is matched against (each node type
	can be capturing, or not).
  Nodes can be composed by various operations, the simplest of which is just
    concatenation.
  Each node must provide the following API:
		std::string regex();
		bool match(std::string);
		static const int NumContained = ...;
		template<int I> auto get();
		template<int I> bool isSet();
		void clear();
  All nodes must handle regex() and match(std::string); this is how the node is
    matched to a target string. regex() should return a string with no captures;
	match(std::string) uses a string with captures instead. This allows composed nodes
	to match using something similar to '(child1->regex())(child2->regex())', ie capturing
	each string that matches the child nodes, and then passing that match to the child
	node to process.
  If the node captures data, then it should do that during the match(std::string) function.
  If the node captures data, then it should set the NumContained value to the number of 
    values it captures; this number is recursive, so if it contains children nodes 
	(like concatenation does) then this value should be the sum of the number of captures
	of each child, as well as any captures it does itself. As this is a static const value,
	for nodes with variable amounts of captured data, they should set this to a value that
	makes sense (so return std::vector<T> for * or +, return std::optional<T> for optional, etc).
  get<I>() and isSet<I>() are for accessing the captured data. For each captured data C_i, where
    i is [0,NumContained), isSet<i>() should return true if the node has captured data, and get<i>()
	should return that data. The type of get<I>() should be dependent on the specific data type
	that is captured.
  A node that captures data must be able to be cleared in in preparation for matching against
    a new string; this prevents old data from being retained if the new string does not capture
	anything for that node (say, for optional data). clear() provides this functionality.
*/

std::string escapeString(std::string s)
{
	std::size_t pos = 0;
	while(pos = s.find_first_of(".()[]|{}*+?^$/-\\",pos))
	{
		if( pos == std::string::npos ) break;
		//if the character before isnt already escaped character, we need to add escape character
		if( pos == 0 or s.at(pos-1) != '\\' )
		{
			s.insert(pos,"\\");
			pos++; //increment, because we just inserted a character!
		}
		pos++; //increment, so we dont match against the character we just matched against
	}
	return s;
}

struct Text {
	std::string text;
	Text(std::string text) : text(escapeString(text)) {}

	static const int NumContained = 0;
	template<int I> void get(); //intentionally undefined
	bool match(std::string s) {
#if PRINT_MATCHES
		std::cout << "matching \"" << s << "\" against \"" << this->regex() << "\"" << std::endl;
#endif
		return RegexCache::instance().match(s, text);
	}
	std::string regex() {return text;}
	void clear(){}
	template<int I> bool isSet(){return false;}
};

template<class T>
struct Variable {
	T value;
	bool is_set;
	Variable() : is_set(false) {}

	static const int NumContained = 1;
	template<int I>
	T get() {
		static_assert(I == 0);
		return value;
	}
	bool match(std::string s) {
#if PRINT_MATCHES
		std::cout << "matching \"" << s << "\" against \"" << this->regex() << "\"" << std::endl;
#endif
		this->clear();
		is_set = RegexCache::instance().match(s, this->regex());
		if( is_set )
		{
			std::stringstream ss(s);
			ss >> value;
		}
		return is_set;
	}
	virtual std::string regex()=0;
	void clear(){is_set=false;value=T{};}
	template<int I>
	bool isSet() {
		static_assert(I == 0);
		return is_set;
	}
};

struct Integer : public Variable<uint64_t> {
	using Variable<uint64_t>::Variable;
	virtual std::string regex() override {return "\\d+";}
};

struct Word : public Variable<std::string> {
	using Variable<std::string>::Variable;
	virtual std::string regex() override {return "\\w+";}
};

struct AllNonWhitespace : public Variable<std::string> {
	using Variable<std::string>::Variable;
	virtual std::string regex() override { return "[^\\s]+";}
};

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
	bool operator == (std::string rhs) const
	{
		return s == rhs;
	}
};

class Keep : public Variable<Line> {
	std::string text;
public:
	Keep(std::string t) : text(t) {}
	virtual std::string regex() override {
		return text;
	}
};

template<class Sub>
struct Repeat {
	Sub sub;
	Text count;
	typedef typename std::conditional<
						Sub::NumContained == 1,
						decltype(std::declval<Sub>().template get<0>()),
						Sub
					>::type ReturnType;
	std::vector<ReturnType> results;
	Repeat(Sub sub, Text count) : sub(sub), count(count) {}
	static const int NumContained = 1;
	template<int I>
	std::vector<ReturnType> get() {
		return results;
	}
	bool match(std::string s)
	{
#if PRINT_MATCHES
		std::cout << "matching \"" << s << "\" against \"" << this->regex() << "\"" << std::endl;
#endif
		this->clear();
		if( !RegexCache::instance().match(s, this->regex()) )
			return false;
		std::regex regex = RegexCache::instance().get(sub.regex());
		auto begin = std::sregex_iterator(s.begin(), s.end(), regex);
		auto end = std::sregex_iterator();
		for(std::sregex_iterator MI = begin; MI != end; ++MI)
		{
			assert( sub.match(MI->str()) );
			if constexpr (Sub::NumContained == 1)
				results.push_back(sub.template get<0>());
			else
				results.push_back(sub);
		}
		return true;
	}
	std::string regex(){return "(?:" + sub.regex() + "){" + count.regex() + "}";} // non-capture
	void clear(){results.clear();}
	template<int I>
	bool isSet(){static_assert(I < NumContained); return true;} //always 'set', even if empty
};

template<class T>
struct isRegex {
	template<class U> static decltype(&U::template get<0>,std::true_type{}) test(int);
	template<class> static std::false_type test(...);
	static constexpr bool value = decltype(test<T>(0))::value;
};

template<class Lhs,
	typename std::enable_if<isRegex<Lhs>::value,bool>::type = true
>
Repeat<Lhs> operator * (Lhs lhs, std::string rhs) {return Repeat<Lhs>(lhs,rhs);}

template<class T,
	typename std::enable_if<isRegex<T>::value,bool>::type = true
>
Repeat<T> operator +(T t) {return Repeat<T>(t, Text{"1,"});}

template<class T,
	typename std::enable_if<isRegex<T>::value,bool>::type = true
>
Repeat<T> operator *(T t) {return Repeat<T>(t,Text{"0,"});}

template<class Lhs, class Rhs>
struct Sum {
	Lhs lhs;
	Rhs rhs;
	Sum(Lhs lhs, Rhs rhs) : lhs(lhs), rhs(rhs) {}
	static const int NumContained = Lhs::NumContained + Rhs::NumContained;
	template<int I>
	auto get()
	{
		if constexpr (I < Lhs::NumContained)
			return lhs.template get<I>();
		else
			return rhs.template get<I-Lhs::NumContained>();
	}
	void clear(){lhs.clear(); rhs.clear();}
	template<int I>
	bool isSet() {
		if constexpr (I < Lhs::NumContained )
			return lhs.template isSet<I>();
		else
			return rhs.template isSet<I-Lhs::NumContained>();
	}
	bool match(std::string s)
	{
#if PRINT_MATCHES
		std::cout << "matching \"" << s << "\" against sum \"" << this->regex() << "\"" << std::endl;
#endif
		this->clear();
		std::smatch sm;
		bool ret = RegexCache::instance().match(s, sm, "("+this->lhs.regex()+")("+this->rhs.regex()+")");
		if( ret )
		{
			assert( sm.size() == 3 );
			this->lhs.match(sm[1]);
			this->rhs.match(sm[2]);
		}
		return ret;
	}
	std::string regex() {return this->lhs.regex() + this->rhs.regex();}
};

template<class Lhs, class Rhs, bool Same=std::is_same<Lhs,Rhs>::value>
struct Or {
	Lhs lhs;
	Rhs rhs;
	Or(Lhs lhs, Rhs rhs) : lhs(lhs), rhs(rhs) {}
	static const int NumContained = Same ? Lhs::NumContained : Lhs::NumContained+Rhs::NumContained;
	template<int I>
	auto get()
	{
		assert( isSet<I>() ); //exactly one must be set
		if constexpr (Same)
		{
			if ( lhs.template isSet<I>() )
				return lhs.template get<I>();
			else
				return rhs.template get<I>();
		}
		else
		{
			if constexpr ( I < Lhs::NumContained )
				return lhs.template get<I>();
			else
				return rhs.template get<I-Lhs::NumContained>();
		}
	}
	void clear(){lhs.clear(); rhs.clear();}
	template<int I>
	bool isSet() {
		assert( I < NumContained );
		if constexpr (Same)
		{
			assert( !(lhs.template isSet<I>() and rhs.template isSet<I>()) ); //only one can be set
			return lhs.template isSet<I>() or rhs.template isSet<I>();
		}
		else
		{
			if constexpr (I < Lhs::NumContained )
				return lhs.template isSet<I>();
			else
				return rhs.template isSet<I-Lhs::NumContained>();
		}
	}
	bool match(std::string s)
	{
#if PRINT_MATCHES
		std::cout << "matching \"" << s << "\" against \"" << this->regex() << "\"" << std::endl;
#endif
		this->clear();
		bool ret = RegexCache::instance().match(s, this->regex());
		if( ret )
		{
			if( RegexCache::instance().match(s, this->lhs.regex()) )
				assert( this->lhs.match(s) );
			else
				assert( this->rhs.match(s) );
		}
		return ret;
	}
	std::string regex() {return "(?:" + this->lhs.regex() + "|" + this->rhs.regex() +")";} // non-capture
};

template<class Lhs, class Rhs,
	typename std::enable_if<isRegex<Lhs>::value,bool>::type = true,
	typename std::enable_if<isRegex<Rhs>::value,bool>::type = true
>
Sum<Lhs,Rhs> operator >> (Lhs lhs, Rhs rhs) {return Sum<Lhs,Rhs>(lhs,rhs);}

template<class Lhs,
	typename std::enable_if<isRegex<Lhs>::value,bool>::type = true
>
Sum<Lhs,Text> operator >> (Lhs lhs, std::string rhs) {return Sum<Lhs,Text>(lhs,rhs);}

template<class Rhs,
	typename std::enable_if<isRegex<Rhs>::value,bool>::type = true
>
Sum<Text,Rhs> operator >> (std::string lhs, Rhs rhs) {return Sum<Text,Rhs>(lhs, rhs);}

template<class Lhs, class Rhs,
	typename std::enable_if<isRegex<Lhs>::value,bool>::type = true,
	typename std::enable_if<isRegex<Rhs>::value,bool>::type = true
>
Or<Lhs,Rhs> operator or (Lhs lhs, Rhs rhs) {return Or<Lhs,Rhs>(lhs,rhs);}

template<class Lhs,
	typename std::enable_if<isRegex<Lhs>::value,bool>::type = true
>
Or<Lhs,Text> operator or (Lhs lhs, std::string rhs) {return Or<Lhs,Text>(lhs,rhs);}

template<class Rhs,
	typename std::enable_if<isRegex<Rhs>::value,bool>::type = true
>
Or<Text,Rhs> operator or (std::string lhs, Rhs rhs) {return Or<Text,Rhs>(lhs, rhs);}

template<int N>
auto MultipleWords()
{
	return Word{} >> Text{" "} >> MultipleWords<N-1>();
}
template<>
auto MultipleWords<1>()
{
	return Word{};
}

template<class Sub>
struct Optional {
	Sub sub;
	Optional(Sub sub) : sub(sub) {}
	static const int NumContained = Sub::NumContained;
	template<int I>
	decltype( std::declval<Sub>().template get<I>() ) get() {
		return sub.template get<I>();
	}
	bool match(std::string s)
	{
#if PRINT_MATCHES
		std::cout << "matching \"" << s << "\" against \"" << this->regex() << "\"" << std::endl;
#endif
		this->clear();
		if( !RegexCache::instance().match(s, this->regex()) )
			return false;
		std::regex regex = RegexCache::instance().get(sub.regex());
		auto begin = std::sregex_iterator(s.begin(), s.end(), regex);
		auto end = std::sregex_iterator();
		if ( begin != end ) {
			assert( sub.match(begin->str()) );
			++begin;
			assert(begin == end);
		}
		return true;
	}
	std::string regex(){return "(?:" + sub.regex() + ")?";} // non-capture
	void clear(){sub.clear();}
	template<int I>
	bool isSet(){return sub.template isSet<I>();}
};

template<class T,
	typename std::enable_if<isRegex<T>::value,bool>::type = true
>
Optional<T> operator &(T t) {return Optional<T>(t);}

template<class T>
class Range : public Variable<T> {
public:
	T min, max;
	Range(T min, T max) : min(min), max(max) {}
	virtual std::string regex() override {
		std::stringstream ret;
		ret << "(?:"; // non-capturing
		for(auto i = min; i <= max; ++i)
		{
			if( i != min ) ret << "|";
			ret << i;
		}
		ret << ")";
		return ret.str();
	}
};

template<class T>
Range<T> range(T min, T max) {return Range<T>(min,max);}

template<class Sub>
class DelimitedList {
	Sub sub;
	std::string delimiter;
	typedef typename std::conditional<
						Sub::NumContained == 1,
						decltype(std::declval<Sub>().template get<0>()),
						Sub
					>::type ReturnType;
	std::vector<ReturnType> results;
public:
	DelimitedList(Sub sub, std::string delimiter) : sub(sub), delimiter(escapeString(delimiter)) {}
	static const int NumContained = 1;
	template<int I>
	std::vector<ReturnType> get() {
		return results;
	}
	bool match(std::string s)
	{
#if PRINT_MATCHES
		std::cout << "matching \"" << s << "\" against \"" << this->regex() << "\"" << std::endl;
#endif
		this->clear();
		auto Reg = sub >> *(Text{delimiter} >> sub);
		if( !Reg.match(s) )
			return false;
		if constexpr (Sub::NumContained == 1)
		{
			results = Reg.template get<1>();
			results.insert(results.begin(), Reg.template get<0>());
		}
		else
		{
			results.push_back(Reg.lhs);
			for(auto M : Reg.template get<Sub::NumContained>()) //M is a match for Text{delimiter} >> sub
				results.push_back(M.rhs);
		}
		return true;
	}
	std::string regex() {auto Reg = sub >> *(Text{delimiter} >> sub); return Reg.regex();}
	void clear(){results.clear();}
	template<int I>
	bool isSet(){return I < results.size();}
};

auto Digit = range(0,9);
auto LowerCase = range('a','z');
auto UpperCase = range('A','Z');
auto Letter = LowerCase or UpperCase;
auto AlphaNum = Digit or Letter;

} /* namespace MyRegex */

#endif /* MY_REGEX_H__ */


