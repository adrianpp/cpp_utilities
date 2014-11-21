#include <iostream>

using namespace std;

/*
range := '{' <numerical> ',' <numerical> ',' <numerical> '}'
used as: {center, max width, min width}
      max width
      []
      ___
     / | \
____/  |  \____
     center
    [  ]
   min width


int val = 6;
int fuzzied0 = apply_rule( rule( on(range(10, 0, 10), 10), 
                                 on(range(20, 0, 10), 30),
                                 on(range(30, 0, 10), 50) ), val);

int fuzzied1 = Rule< On<Range<10,0,10>, 10>,
                     On<Range<20,0,10>, 30>,
                     On<Range<30,0,10>, 50> >::apply(val);

*/

template<class T>
class DRange {
public:
  typedef T ValueType;
private:
  ValueType center;
  ValueType maxWidth;
  ValueType minWidth;
public:
  ValueType getCenter() {return center;}
  ValueType getMaxWidth() {return maxWidth;}
  ValueType getMinWidth() {return minWidth;}
  DRange(ValueType _center, ValueType _maxWidth, ValueType _minWidth) : center(_center), maxWidth(maxWidth), minWidth(_minWidth) {}
};

template<int Center, int MaxWidth, int MinWidth>
class CRange {
public:
  typedef int ValueType;
  const static ValueType getCenter() {return Center;}
  const static ValueType getMaxWidth() {return MaxWidth;}
  const static ValueType getMinWidth() {return MinWidth;}
  CRange() {}
	typedef double WeightType;
	const static WeightType getWeight(ValueType x)
	{
		if( x <= getCenter() - getMinWidth() or
		    x >= getCenter() + getMinWidth() )
			return 0.0;
		if( x >= getCenter() - getMaxWidth() and
			  x <= getCenter() + getMaxWidth() )
			return 1.0;
		//y=ax+b
		//a=(y1-y0)/(x1-x0)
		//b=(y0*x1-x0*y1)/(x1-x0)
		//with y1=1,y0=0:
		//y=(x-x0)/(x1-x0)
		if( x < getCenter() )
		{
			const ValueType x0 = getCenter() - getMinWidth();
			const ValueType x1 = getCenter() - getMaxWidth();
			return (WeightType(x-x0)/(x1-x0));
		}
		if( x > getCenter() )
		{
			const ValueType x0 = getCenter() + getMinWidth();
			const ValueType x1 = getCenter() + getMaxWidth();
			return (WeightType(x-x0)/(x1-x0));
		}
	  throw "";
	}
};

template<class T>
DRange<T> range(T center, T maxWidth, T minWidth)
{
  return DRange<T>(center, maxWidth, minWidth);
}

template<int A, int B, int C>
CRange<A,B,C> range()
{
  return CRange<A,B,C>();
}

template<int A, int B, int C>
class Range : public CRange<A,B,C> {};

template<class R, class V>
class DRangeOnValue {
public:
  typedef R RangeType;
  typedef V ValueType;
private:
    RangeType range;
    ValueType value;
public:
  RangeType getRange() {return range;}
  ValueType getValue() {return value;}
  DRangeOnValue(RangeType _range, ValueType _value) : range(_range), value(_value) {}
};

template<class R, int v>
class CRangeOnValue {
public:
  typedef R RangeType;
  typedef int ValueType;
public:
  static RangeType getRange() {return RangeType();}
  static ValueType getValue() {return v;}
  static ValueType getWeightedVal(ValueType val)
	{
		return RangeType::getWeight(val) * getValue();
	}
  CRangeOnValue() {}
};

template<class R, class V>
DRangeOnValue<R,V> on(R range, V value)
{
  return DRangeOnValue<R,V>(range, value);
}

template<class R, int v>
CRangeOnValue<R,v> on()
{
    return CRangeOnValue<R,v>();
}

template<class R, int v>
class On : public CRangeOnValue<R,v> {};

class Empty {
public:
  class EmptyRange {
	public:
	  template<class T>
	  static const float getWeight(T) {return 0;}
	};
  typedef EmptyRange RangeType;
	template<class T>
	static const T getWeightedVal(T) {return 0;}
};

template<class _On0=Empty, class _On1=Empty, class _On2=Empty, class _On3=Empty, class _On4=Empty, class _On5=Empty>
class CRule {
public:
  typedef _On0 On0;
  typedef _On1 On1;
  typedef _On2 On2;
  typedef _On3 On3;
  typedef _On4 On4;
  typedef _On5 On5;
public:
  template<class T>
  static T apply(T val)
  {
		  T summedWeight = On0::RangeType::getWeight(val) + On1::RangeType::getWeight(val) + On2::RangeType::getWeight(val) + 
			                 On3::RangeType::getWeight(val) + On4::RangeType::getWeight(val) + On5::RangeType::getWeight(val);
			T summedValue  = On0::getWeightedVal(val) + On1::getWeightedVal(val) + On2::getWeightedVal(val) +
			                 On3::getWeightedVal(val) + On4::getWeightedVal(val) + On5::getWeightedVal(val);
			if( summedWeight != 0 )
        return summedValue / summedWeight;
			std::cerr << "Rule does not have coverage!\n";
			return 0;
  }
};

int main()
{
   typedef Range<0x00, 0x00, 0x10> cblack;
   typedef Range<0x20, 0x00, 0x18> cdark;
   typedef Range<0x40, 0x00, 0x20> cdim;
   typedef Range<0x90, 0x00, 0x50> cbright;
   typedef Range<0xF0, 0x10, 0x50> clight;
   
   typedef CRule< On<cblack, 0x00>,
                  On<cdark,  0xA0>,
                  On<cdim,   0xC0>,
                  On<cbright,0xF0>,
                  On<clight, 0xFF> > MoveLight;

    for(float i = 0; i < 255; ++i)
		  std::cout << MoveLight::apply(i) << std::endl;
                  
   return 0;
}
