/*
 * decimal.hpp
 *
 *  Created on: 26/03/2010
 *      Author: alan.lohse
 */

#ifndef DECIMAL_HPP_
#define DECIMAL_HPP_

#include <utils/funcs.hpp>
#include <algorithm>
#include <utils/math/giantint.hpp>

namespace utils {

namespace math {

class decimal {
public:
	enum ROUND_MODE {
		ROUND_HALF_UP = 0,
		ROUND_UP = 1,
		ROUND_DOWN = 2,
		ROUND_FLOOR = 3,
		ROUND_CEIL = 4
	};
private:
	giantint _M_int_value;
	t_short _M_scale;

	decimal do_add(const decimal& other) const {
		int align1 = 0, align2 = 0;
		giantint scalev1(1);
		giantint scalev2(1);
		align1 = other._M_scale - _M_scale;
		align2 = _M_scale - other._M_scale;
		while (align1-- > 0) scalev1.single_mul(10);
		while (align2-- > 0) scalev2.single_mul(10);
		giantint p1 = (*this)._M_int_value * scalev1;
		giantint p2 = other._M_int_value * scalev2;
		giantint int_result(p1+p2);
		decimal result(int_result,std::max(_M_scale, other._M_scale));
		return result;
	}
	decimal do_mul(const decimal& other) const {
		giantint int_result(_M_int_value*other._M_int_value);
		decimal result(int_result,_M_scale + other._M_scale);
		return result;
	}
	decimal do_div(const decimal& other, t_short scale, ROUND_MODE mode) const {
		giantint int_rem;
		giantint scaledvd(1);
		int scale_add = other._M_int_value.digits()+1+(scale > 0 ? scale : 0);
		if (_M_scale < 0) {
			int scdvd = -_M_scale;
			while (scdvd-- > 0) scaledvd.single_mul(10);
		}
		if (scale_add > 0){
			int scdvd = scale_add;
			while (scdvd-- > 0) scaledvd.single_mul(10);
		}
		giantint int_result(_M_int_value*scaledvd);
		int_result = int_result.divide_and_remainder(other._M_int_value,int_rem);

		decimal result(int_result,_M_scale + scale_add - other._M_scale);
		if (scale >= 0)
			return result.scale(scale, mode);
		else
			return result.scale(_M_scale + scale_add - other._M_scale-1, mode);
	}
public:
	static const decimal zero() {
		static const decimal _zero;
		return _zero;
	}
	static const decimal one(){
		static const decimal _one(1,0);
		return _one;
	}
	static const decimal ten() {
		static const decimal _ten(10,0);
		return _ten;
	}
public:
	decimal():
		_M_int_value(),
		_M_scale(0)
	{
	}

	decimal(const giantint& int_value, const t_short scale):
		_M_int_value(int_value),
		_M_scale(scale)
	{
	}

	decimal(const t_float d):
		_M_int_value(),
		_M_scale()
	{
		char tmp[32];
		::sprintf(tmp,"%-+#.*f",7,d);
		int length = ::strlen(tmp);
		for (int i = length -1; i>= 0; i--) {
			if (tmp[i] == '0') {
				length--;
				tmp[i] = 0;
			} else if (tmp[i] == '.') {
				length--;
				tmp[i] = 0;
				break;
			} else {
				break;
			}
		}
		for (int i = length -1; i>= 0; i--)
			if (tmp[i] == '.') {
				_M_scale = length-i-1;
				break;
			}
		for (int i = length - _M_scale -1; i < length && _M_scale; i++)
			tmp[i] = tmp[i+1];
		_M_int_value = giantint(tmp);
	}

	decimal(const t_int d, t_short scale):
		_M_int_value(d),
		_M_scale(scale)
	{
	}

	decimal(const t_bigint d, t_short scale):
		_M_int_value(d),
		_M_scale(scale)
	{
	}

	decimal(const t_double d):
		_M_int_value(),
		_M_scale()
	{
		char tmp[48];
		::sprintf(tmp,"%-+#.*f",16,d);
		int length = ::strlen(tmp);
		for (int i = length -1; i>= 0; i--) {
			if (tmp[i] == '0') {
				length--;
				tmp[i] = 0;
			} else if (tmp[i] == '.') {
				length--;
				tmp[i] = 0;
				break;
			} else {
				break;
			}
		}
		for (int i = length -1; i>= 0; i--)
			if (tmp[i] == '.') {
				_M_scale = length-i-1;
				break;
			}
		for (int i = length - _M_scale - 1; i < length && _M_scale; i++)
			tmp[i] = tmp[i+1];
		_M_int_value = giantint(tmp);
	}

	decimal(const std::string& str):
		_M_int_value(),
		_M_scale(0)
	{
		int length = str.length();
		for (int i = length -1; i>= 0; i--)
			if (str[i] == '.') {
				_M_scale = length-i-1;
				break;
			}
		std::string str2 = str;
		if (_M_scale > 0) {
			str2.erase(str2.begin()+(length-_M_scale-1));
		}
		_M_int_value = giantint(str2);
	}

	decimal(const decimal& other):
		_M_int_value(other._M_int_value),
		_M_scale(other._M_scale)
	{
	}

	bool equal_to(const decimal& other) const {
		if (_M_scale == other._M_scale)
			return _M_int_value == other._M_int_value;
		else {
			int align1 = 0, align2 = 0;
			giantint scalev1(1);
			giantint scalev2(1);
			align1 = other._M_scale - _M_scale;
			align2 = _M_scale - other._M_scale;
			while (align1-- > 0) scalev1.single_mul(10);
			while (align2-- > 0) scalev2.single_mul(10);
			giantint p1 = (*this)._M_int_value * scalev1;
			giantint p2 = other._M_int_value * scalev2;
			return p1 == p2;
		}
	}

	t_int compare_to(const decimal& other) const {
		if (_M_scale == other._M_scale)
			return _M_int_value.compare_to(other._M_int_value);
		else {
			int align1 = 0, align2 = 0;
			giantint scalev1(1);
			giantint scalev2(1);
			align1 = other._M_scale - _M_scale;
			align2 = _M_scale - other._M_scale;
			while (align1-- > 0) scalev1.single_mul(10);
			while (align2-- > 0) scalev2.single_mul(10);
			giantint p1 = (*this)._M_int_value * scalev1;
			giantint p2 = other._M_int_value * scalev2;
			return p1.compare_to(p2);
		}
	}

	decimal add(const decimal& other) const {
		return do_add(other);
	}

	decimal subtract(const decimal& other) const {
		return do_add(other.minus());
	}

	decimal multiply(const decimal& other) const {
		return do_mul(other);
	}

	decimal divide(const decimal& other) const {
		return do_div(other,-1,ROUND_HALF_UP);
	}

	decimal divide(const decimal& other, const t_short scale, const ROUND_MODE mode) const {
		return do_div(other,scale,mode);
	}

	decimal minus() const {
		decimal r(*this);
		r._M_int_value._M_signal *= -1;
		return r;
	}

	decimal frac() const {
		decimal b = *this;
		b.scale(0);
		return *this-b;
	}

	decimal sqrt(const t_short scale = -1, const ROUND_MODE mode = ROUND_HALF_UP) const {
		int log10 = 0;
		decimal x = *this;
		// calcules a aproximated value
		// using the expression sqrt(x) = 2^(log2(x)/2)
		while (x > ten()){
			x._M_int_value.single_div(10);
			log10++;
		}
		int prec = log10+x._M_scale;
		if (scale > 0)
			prec = prec - x._M_scale + scale;
		log10/=2;
		while (log10>0){
			x._M_int_value.single_mul(10);
			log10--;
		}
		// now refines it
		static decimal half("0.5");
		for (int i = 0; i < prec + 1; i++)
			x = half*(x + (*this)/x);
		if (scale < 0)
			return x.scale(_M_scale,mode);
		else
			return x.scale(scale,mode);
	}

	decimal pow(t_int power) const {
		decimal r = decimal(1,0);
		while (power-- > 0) {
			r *= *this;
		}
		return r;
	}

	decimal abs() const {
		decimal r(*this);
		r._M_int_value._M_signal = 1;
		return r;
	}

	bool is_zero() const {
		return _M_int_value.is_zero();
	}

	std::string to_string() const {
		t_int dec = 0, sign = 0;
		std::string r = to_string(dec,sign);
		if (dec != r.length())
			r.insert(r.begin()+dec,'.');
		if (sign == -1)
			r.insert(r.begin()+dec,'-');
		return r;
	}

	std::string to_string(t_int& dec, t_int& sign) const {
		std::string r(_M_int_value.to_string(10,sign));
		std::string xzeros;
		dec = r.length() - _M_scale;
		if (dec < 1) {
			for (int i = 0; i < 1-dec;i++) {
				xzeros+='0';
			}
			dec = 1;
		}
		return xzeros+r;
	}

	t_int to_int() const {
		if (_M_scale != 0) {
			decimal gi(*this);
			gi.scale(0,ROUND_HALF_UP);
			return gi._M_int_value.to_int();
		} else {
			return _M_int_value.to_int();
		}
	}

	t_bigint to_bigint() const {
		if (_M_scale != 0) {
			decimal gi(*this);
			gi.scale(0,ROUND_HALF_UP);
			return gi._M_int_value.to_bigint();
		} else {
			return _M_int_value.to_bigint();
		}
	}

	t_float to_float() const {
		t_float r = _M_int_value.to_float();
		t_int s = _M_scale;
		while(s-- >0) r/=10;
		return r;
	}

	t_double to_double() const {
		t_double r = _M_int_value.to_double();
		t_int s = _M_scale;
		while(s-- >0) r/=10;
		return r;
	}

	/**
	 * Returns the scale of this decimal.
	 * The scale is a positive integer that represents the number of digits
	 * to the right of the decimal point.
	 * \return the scale of this decimal
	 */
	t_short scale() const {
		return _M_scale;
	}

	/**
	 * Set the scale of this decimal
	 */
	decimal& scale(t_short s, const ROUND_MODE mode = ROUND_HALF_UP) {
		t_short ds = _M_scale - s;
		if (ds > 0) {
			_M_scale = s;
			for (; ds != 0; ds--) {
				if (ds == 1) {
					t_dword m = _M_int_value.single_mod(10);
					switch (mode) {
						case ROUND_HALF_UP:
							if (m >= 5) m = 10;
						break;
						case ROUND_UP:
							if (m != 0) m = 10;
						break;
						case ROUND_DOWN:
							m = 0;
						break;
						case ROUND_FLOOR:
							if (m != 0) {
								if (_M_int_value._M_signal == -1)
									m = 10;
								else m = 0;
							}
						break;
						case ROUND_CEIL:
							if (m != 0) {
								if (_M_int_value._M_signal == 1)
									m = 10;
								else m = 0;
							}
						break;
					}
					_M_int_value.single_add(m);
				}
				_M_int_value.single_div(10);
			}
		} else {
			_M_scale = s;
			for (; ds != 0; ds++) {
				_M_int_value.single_mul(10);
			}
		}
		return *this;
	}

	decimal operator+(const decimal& other) const {
		return add(other);
	}

	decimal operator-(const decimal& other) const {
		return subtract(other);
	}

	decimal operator*(const decimal& other) const {
		return multiply(other);
	}

	decimal operator/(const decimal& other) const {
		return divide(other);
	}

	decimal operator-() const {
		return minus();
	}

	decimal operator+() const {
		return *this;
	}

	decimal& operator=(const decimal& other) {
		_M_int_value = other._M_int_value;
		_M_scale = other._M_scale;
		return *this;
	}

	decimal& operator+=(const decimal& other) {
		return *this = add(other);
	}

	decimal& operator-=(const decimal& other) {
		return *this = subtract(other);
	}

	decimal& operator*=(const decimal& other) {
		return *this = multiply(other);
	}

	decimal& operator/=(const decimal& other) {
		return *this = divide(other);
	}

	bool operator == (const decimal& other) const {
		return equal_to(other);
	}

	bool operator != (const decimal& other) const {
		return !equal_to(other);
	}

	bool operator > (const decimal& other) const {
		return compare_to(other) > 0;
	}

	bool operator >= (const decimal& other) const {
		return compare_to(other) >= 0;
	}

	bool operator < (const decimal& other) const {
		return compare_to(other) < 0;
	}

	bool operator <= (const decimal& other) const {
		return compare_to(other) <= 0;
	}

};

inline std::ostream& operator<<(std::ostream& os, const decimal& d) {
	os << d.to_string();
	return os;
}

}

}

#endif /* DECIMAL_HPP_ */
