#include "VariableValue.hpp"
#include <iostream>

VariableValue::VariableValue(int _width) : bits(_width), width(_width)
{
}

VariableValue::VariableValue(std::vector<bool> _bits) : bits(_bits), width(bits.size())
{
}

VariableValue::VariableValue(const VariableValue& rhs) : bits(rhs.bits), width(rhs.width)
{
}

int VariableValue::getWidth() const
{
	return width;
}

VariableValue VariableValue::trimToWidth(int w) const
{
	assert( w <= width && "Trim width cannot be larger than original width!" );
	VariableValue rhs(*this);
	rhs.bits.resize(w);
	rhs.width = w;
	return rhs;
}

VariableValue VariableValue::extendToWidth(int w) const
{
	assert( w >= width && "Extend width cannot be smaller than original width!" );
	VariableValue rhs(*this);
	rhs.bits.resize(w);
	rhs.width = w;
	return rhs;
}

VariableValue VariableValue::resize(int w) const
{
	if( w < width )
		return trimToWidth(w);
	if( w > width )
		return extendToWidth(w);
	return *this;
}

bool VariableValue::getBit(int n) const
{
	if( n >= width )
	{
		std::cerr << "Trying to get bit " << n << " from a " << width << " bit wide value!\n";
	}
	return bits.at(n);
}

VariableValue VariableValue::create_from_int(const int w, int v)
{
	std::vector<bool> b;
	for(int c = 0; c < w; ++c)
	{
		b.push_back( v & 1 );
		v >>= 1;
	}
   VariableValue ret(b);
	return ret;
}

std::ostream& operator << (std::ostream& os, const VariableValue& rhs)
{
	if( os.flags() & std::ios::hex ) //in hex mode, display as hex
	{
		const int NibbleLength = 4;
		int needed_nibbles = rhs.getWidth() / NibbleLength;
		if( rhs.getWidth() % NibbleLength > 0 ) //if it doesnt divide easily, we need another nibble
			++needed_nibbles;
		for(int nibbleNum = needed_nibbles - 1; nibbleNum >= 0; --nibbleNum)
		{
      int nibbleVal = 0;
			for(int bitNum = NibbleLength-1; bitNum >= 0; --bitNum)
			{
				int finalBitIndex = nibbleNum * NibbleLength + bitNum;
				bool bitVal = 0;
				if( finalBitIndex < rhs.getWidth() )
					bitVal = rhs.getBit(finalBitIndex);
				nibbleVal = (nibbleVal << 1) | bitVal;
			}
			os << nibbleVal;
		}
	}
	else //in other modes, display as binary
	{
	  for(int w = rhs.getWidth() - 1; w >= 0; --w)
	  {
	  	os << rhs.getBit(w);
	  }
	}
	return os;
}

bool operator != (const VariableValue& lhs, const VariableValue& rhs)
{
	int width = lhs.getWidth();
	if( width < rhs.getWidth() )
		width = rhs.getWidth();	
	VariableValue lhs_resize = lhs.resize(width);
	VariableValue rhs_resize = rhs.resize(width);

	//go most significant -> least significant
	for(int c = width-1; c >= 0; --c)
	{
		if( lhs_resize.getBit(c) != rhs_resize.getBit(c) )
			return true;
	}
	return false;
}

bool operator < (const VariableValue& lhs, const VariableValue& rhs)
{
	int width = lhs.getWidth();
	if( width < rhs.getWidth() )
		width = rhs.getWidth();	
	VariableValue lhs_resize = lhs.resize(width);
	VariableValue rhs_resize = rhs.resize(width);
	//go most significant -> least significant
	for(int c = width-1; c >= 0; --c)
	{
		if( lhs_resize.getBit(c) < rhs_resize.getBit(c) )
			return true;
		if( lhs_resize.getBit(c) > rhs_resize.getBit(c) )
			return false;
	}
	return false;
}

bool operator > (const VariableValue& lhs, const VariableValue& rhs)
{
	int width = lhs.getWidth();
	if( width < rhs.getWidth() )
		width = rhs.getWidth();	
	VariableValue lhs_resize = lhs.resize(width);
	VariableValue rhs_resize = rhs.resize(width);
	//go most significant -> least significant
	for(int c = width-1; c >= 0; --c)
	{
		if( lhs_resize.getBit(c) > rhs_resize.getBit(c) )
			return true;
		if( lhs_resize.getBit(c) < rhs_resize.getBit(c) )
			return false;
	}
	return false;
}
bool operator == (const VariableValue& lhs, const VariableValue& rhs)
{
  return !( lhs != rhs );
}

bool operator <= (const VariableValue& lhs, const VariableValue& rhs)
{
	return !(lhs > rhs);
}

bool operator >= (const VariableValue& lhs, const VariableValue& rhs)
{
	return !(lhs < rhs);
}

bool signed_greater(const VariableValue& lhs, const VariableValue& rhs)
{
	bool lhsNegative = lhs.getBit(lhs.getWidth()-1);
	bool rhsNegative = rhs.getBit(rhs.getWidth()-1);
	if( lhsNegative and !rhsNegative )
    return false;
	if( !lhsNegative and rhsNegative )
		return true;
	//none of those conditions, we are same sign
	return (lhs > rhs);
}

bool signed_less(const VariableValue& lhs, const VariableValue& rhs)
{
	bool lhsNegative = lhs.getBit(lhs.getWidth()-1);
	bool rhsNegative = rhs.getBit(rhs.getWidth()-1);
	if( lhsNegative and !rhsNegative )
    return true;
	if( !lhsNegative and rhsNegative )
		return false;
	//none of those conditions, we are same sign
	return (lhs < rhs);
}

VariableValue operator + (const VariableValue& src1, const VariableValue& src2)
{
	assert( src1.getWidth() == src2.getWidth() and "Cannot add values of different widths!" );
	int maxSize = src1.getWidth();
	bool carry = 0;
  std::vector<bool> result;
	for(int b = 0; b < maxSize; ++b)
	{
		int temp = carry + src1.getBit(b) + src2.getBit(b);
		carry = (temp > 1);
		if( carry )
			temp -= 2;
		result.push_back(temp);
	}
	return VariableValue(result);
}

VariableValue operator - (const VariableValue& src1, const VariableValue& src2)
{
	assert( src1.getWidth() == src2.getWidth() and "Cannot subtract values of different widths!" );
	int maxSize = src1.getWidth();
	bool borrow = 0;
  std::vector<bool> result;
	for(int b = 0; b < maxSize; ++b)
	{
		int temp = src1.getBit(b) - src2.getBit(b) - borrow;
		borrow = (temp < 0);
		if( borrow )
			temp += 2;
		result.push_back(temp);
	}
	return VariableValue(result);
}

VariableValue operator << (const VariableValue& src1, const VariableValue& shft)
{
	VariableValue zero = VariableValue::create_from_int(shft.getWidth(), 0);
	VariableValue one  = VariableValue::create_from_int(shft.getWidth(), 1);
	std::vector<bool> result;
	for(VariableValue c(shft); c != zero; c = c - one)
	{
    result.push_back(0);
	}
	for(int c = 0; c < src1.getWidth(); ++c)
	{
		result.push_back(src1.getBit(c));
	}
	return VariableValue(result).resize(src1.getWidth());
}

VariableValue operator >> (const VariableValue& src1, const VariableValue& shft)
{
	VariableValue zero = VariableValue::create_from_int(shft.getWidth(), 0);
	VariableValue one  = VariableValue::create_from_int(shft.getWidth(), 1);
	std::vector<bool> result;
  for(int c = 0; c < src1.getWidth(); ++c)
	{
		result.push_back(src1.getBit(c));
	}
	for(VariableValue c(shft); c != zero; c = c - one)
	{
		result.erase(result.begin());
    result.push_back(0);
	}
	return VariableValue(result).resize(src1.getWidth());
}

VariableValue signed_right_shift(const VariableValue& src1, const VariableValue& shft)
{
	VariableValue zero = VariableValue::create_from_int(shft.getWidth(), 0);
	VariableValue one  = VariableValue::create_from_int(shft.getWidth(), 1);
	bool signBit = src1.getBit(src1.getWidth()-1);
	std::vector<bool> result;
  for(int c = 0; c < src1.getWidth(); ++c)
	{
		result.push_back(src1.getBit(c));
	}
	for(VariableValue c(shft); c != zero; c = c - one)
	{
		result.erase(result.begin());
    result.push_back(signBit);
	}
	return VariableValue(result).resize(src1.getWidth());
}

VariableValue operator * (const VariableValue& src1, const VariableValue& src2)
{
  assert( src1.getWidth() == src2.getWidth() and "Cannot multiply values of different widths!" );
	int width = src1.getWidth();
  std::vector<bool> result;
	int carry = 0;
	for(int n = 0; n < width; ++n)
  {
		int R_n = carry;
		for(int j = 0; j <= n; ++j)
		{
			R_n += src1.getBit(j) and src2.getBit(n-j);
		}
		carry = R_n / 2;
		result.push_back(R_n % 2);
	}
	return VariableValue(result);
}

VariableValue operator & (const VariableValue& src1, const VariableValue& src2)
{
  assert( src1.getWidth() == src2.getWidth() and "Cannot and values of different widths!" );
	int width = src1.getWidth();
  std::vector<bool> result;
	for(int n = 0; n < width; ++n)
  {
		result.push_back(src1.getBit(n) and src2.getBit(n));
	}
	return VariableValue(result);
}

VariableValue operator | (const VariableValue& src1, const VariableValue& src2)
{
  assert( src1.getWidth() == src2.getWidth() and "Cannot or values of different widths!" );
	int width = src1.getWidth();
  std::vector<bool> result;
	for(int n = 0; n < width; ++n)
  {
		result.push_back(src1.getBit(n) or src2.getBit(n));
	}
	return VariableValue(result);
}

VariableValue operator ^ (const VariableValue& src1, const VariableValue& src2)
{
  assert( src1.getWidth() == src2.getWidth() and "Cannot xor values of different widths!" );
	int width = src1.getWidth();
  std::vector<bool> result;
	for(int n = 0; n < width; ++n)
  {
		result.push_back(src1.getBit(n) xor src2.getBit(n));
	}
	return VariableValue(result);
}

VariableValue operator ~ (const VariableValue& src1)
{
	int width = src1.getWidth();
  std::vector<bool> result;
	for(int n = 0; n < width; ++n)
  {
		result.push_back(!src1.getBit(n));
	}
	return VariableValue(result);
}

VariableValue extractBits(const VariableValue& src1, int startBit, int endBit)
{
	std::vector<bool> result;
	for(int n = startBit; n < endBit; ++n)
	{
		result.push_back(src1.getBit(n));
	}
	return VariableValue(result);
}
