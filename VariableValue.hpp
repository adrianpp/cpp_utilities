#ifndef _VARIABLE_VALUE_HPP__
#define _VARIABLE_VALUE_HPP__

#include <vector>
#include <ostream>
#include <assert.h>

//Bit 0 is least significant bit, bit N-1 is most significant bit, where width = N
class VariableValue {
	std::vector<bool> bits;
	int width;
public:
  explicit VariableValue(int _width);
	explicit VariableValue(std::vector<bool> _bits);
	VariableValue(const VariableValue& rhs);
	int getWidth() const;
	VariableValue trimToWidth(int w) const;
	VariableValue extendToWidth(int w) const;
	VariableValue resize(int w) const;	
	bool getBit(int n) const;
	static VariableValue create_from_int(const int w, int v);
	template<class T> T convertTo() const
	{
		assert( getWidth() == sizeof(T) * 8 and "Cannot convert differing sized values!" );
		T ret(0);
		for(int c = getWidth()-1; c >= 0; --c)
		{
			ret = (ret << 1) | getBit(c);
		}
		return ret;
	}
};

std::ostream& operator << (std::ostream& os, const VariableValue& rhs);

bool operator != (const VariableValue& lhs, const VariableValue& rhs);
bool operator < (const VariableValue& lhs, const VariableValue& rhs);
bool operator > (const VariableValue& lhs, const VariableValue& rhs);
bool operator == (const VariableValue& lhs, const VariableValue& rhs);
bool operator <= (const VariableValue& lhs, const VariableValue& rhs);
bool operator >= (const VariableValue& lhs, const VariableValue& rhs);
bool signed_greater(const VariableValue& lhs, const VariableValue& rhs);
bool signed_less(const VariableValue& lhs, const VariableValue& rhs);

VariableValue operator + (const VariableValue& src1, const VariableValue& src2);
VariableValue operator - (const VariableValue& src1, const VariableValue& src2);
VariableValue operator << (const VariableValue& src1, const VariableValue& shft);
VariableValue operator >> (const VariableValue& src1, const VariableValue& shft);
VariableValue signed_right_shift(const VariableValue& src1, const VariableValue& shft);
VariableValue operator * (const VariableValue& src1, const VariableValue& src2);
VariableValue operator & (const VariableValue& src1, const VariableValue& src2);
VariableValue operator | (const VariableValue& src1, const VariableValue& src2);
VariableValue operator ^ (const VariableValue& src1, const VariableValue& src2);
VariableValue operator ~ (const VariableValue& src1);

VariableValue extractBits(const VariableValue& src1, int startBit, int endBit);

#endif
