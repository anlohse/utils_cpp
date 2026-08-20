/*
 * giantint.hpp
 *
 *  Created on: 26/03/2010
 *      Author: alan.lohse
 */

#ifndef GIANTINT_HPP_
#define GIANTINT_HPP_

#include <utils/utils_defs.hpp>
#include <algorithm>
#include <utils/funcs.hpp>
#include <utils/math/math_exception.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <memory>

namespace utils {

namespace math {

class giantint {
public:
	typedef std::vector<t_dword, DEFAULT_ALLOCATOR<t_dword>  > array_type;
private:
	friend class decimal;
	array_type data;
	t_small _M_signal;

	void normalize() {
		while (data.size() != 0 && data.back() == 0)
			data.pop_back();
	}

	giantint& single_div(t_dword d) {
		t_dword rem = 0;
		array_type::reverse_iterator it = data.rbegin();
		array_type::reverse_iterator end = data.rend();
		for (; it != end; it++) {
			t_qword a = (t_qword)rem << 32 | (t_qword)(*it);
			t_qword r = a/((t_qword)d);
			rem = a%((t_qword)d);
			*it = (t_dword)r;
		}
		normalize();
		return *this;
	}
	t_dword single_mod(const t_dword d) const {
		t_dword rem = 0;
		array_type::const_reverse_iterator it = data.rbegin();
		array_type::const_reverse_iterator end = data.rend();
		for (; it != end; it++) {
			t_qword a = (t_qword)rem << 32 | (t_qword)(*it);
			rem = a%((t_qword)d);
		}
		return rem;
	}
	giantint& single_mul(t_dword d) {
		t_dword carry = 0;
		array_type::iterator it = data.begin();
		array_type::iterator end = data.end();
		for (; it != end; it++) {
			t_dword a = *it;
			t_qword r = (t_qword)a * (t_qword)d + (t_qword)carry;
			*it = (t_dword)r;
			carry = (t_dword)(r >> 32);
		}
		if (carry != 0)
			data.push_back(carry);
		normalize();
		return *this;
	}
	giantint& single_add(t_dword d) {
		t_dword carry = d;
		array_type::iterator it = data.begin();
		array_type::iterator end = data.end();
		for (; it != end && carry != 0; it++) {
			t_dword a = *it;
			t_qword r = (t_qword)a+(t_qword)carry;
			*it = (t_dword)r;
			carry = (t_dword)(r >> 32);
		}
		if (carry != 0)
			data.push_back(carry);
		return *this;
	}
	giantint& do_lshift(t_dword l) {
		t_dword carry = 0;
		while(l > 32) {
			data.insert(data.begin(),(size_t)1,(t_dword)0);
			l -= 32;
		}
		array_type::iterator it = data.begin();
		array_type::iterator end = data.end();
		for (; it != end; it++) {
			t_qword r = ((t_qword)(*it) << l) | (t_qword)carry;
			*it = (t_dword)r;
			carry = (t_dword)(r >> 32);
		}
		if (carry != 0)
			data.push_back(carry);
		return *this;
	}
	giantint& do_rshift(t_dword l) {
		t_dword last = 0;
		while(l > 32) {
			data.erase(data.begin());
			if (data.size() == 0) {
				_M_signal = 1;
				return *this;
			}
			l -= 32;
		}
		array_type::reverse_iterator it = data.rbegin();
		array_type::reverse_iterator end = data.rend();
		for (; it != end; it++) {
			t_qword a = (t_qword)*it << 32;
			t_qword r = ((t_qword)a >> l) | ((t_qword)last << 32);
			*it = (t_dword)(r >> 32);
			last = (t_dword)r;
		}
		normalize();
		return *this;
	}
	giantint& do_or(t_dword l) {
		if (data.size() != 0)
			data[0] |= l;
		else
			data.push_back(l);
		return *this;
	}
	giantint& do_and(t_dword l) {
		if (data.size() != 0)
			data[0] &= l;
		normalize();
		return *this;
	}
	giantint& do_xor(t_dword l) {
		if (data.size() != 0)
			data[0] ^= l;
		normalize();
		return *this;
	}
	giantint& do_or(const giantint& other) {
		t_size length = std::min(data.size(), other.data.size());
		for (t_size i = 0; i < length; i++) {
			data[i] |= other.data[i];
		}
		return *this;
	}
	giantint& do_and(const giantint& other) {
		for (t_size i = 0; i < data.size(); i++) {
			data[i] &= i < other.data.size() ? other.data[i] : 0;
		}
		normalize();
		return *this;
	}
	giantint& do_xor(const giantint& other) {
		for (t_size i = 0; i < data.size(); i++) {
			data[i] ^= i < other.data.size() ? other.data[i] : 0;
		}
		normalize();
		return *this;
	}
	giantint& do_not() {
		for (t_size i = 0; i < data.size(); i++) {
			data[i] = ~data[i];
		}
		normalize();
		return *this;
	}
	giantint do_add(const giantint& other) const {
		giantint result;
		t_dword carry = 0;
		t_size length = std::max(data.size(), other.data.size());
		for (t_size i = 0; i < length; i++) {
			t_dword a = i < data.size() ? data[i] : 0;
			t_dword b = i < other.data.size() ? other.data[i] : 0;
			t_qword r = (t_qword)a+(t_qword)b+(t_qword)carry;
			result.data.push_back((t_dword)r);
			carry = (t_dword)(r >> 32);
		}
		if (carry != 0)
			result.data.push_back(carry);
		result._M_signal = _M_signal;
		return result;
	}
	giantint difference(const giantint& other) const {
		giantint result;
		t_dword borrow = 0;
		t_size length = std::max(data.size(), other.data.size());
		for (t_size i = 0; i < length; i++) {
			t_dword a = i < data.size() ? data[i] : 0;
			t_dword b = i < other.data.size() ? other.data[i] : 0;
			t_dword obw = borrow;
			if ((a-obw) < b)
				borrow = 1;
			else
				borrow = 0;
			t_qword bw = ((t_qword)borrow) << 32;
			t_qword r = (t_qword)a + bw - obw -(t_qword)b;
			result.data.push_back((t_dword)r);
		}
		result._M_signal = _M_signal;
		result.normalize();
		return result;
	}
	giantint do_subtract(const giantint& other) const {
		t_int cmp = compare_to(other)*_M_signal;
		if (cmp == 1) {
			giantint result = difference(other);
			result._M_signal = _M_signal;
			return result;
		} else if (cmp == -1) {
			giantint result = other.difference(*this);
			result._M_signal = -_M_signal;
			return result;
		}
		return giantint();
	}
	giantint do_multiply(const giantint& other) const {
		giantint result;
		if (!is_zero() && !other.is_zero()) {
			for (size_t i = 0; i < other.data.size(); i++) {
				array_type::const_iterator it = data.begin();
				array_type::const_iterator end = data.end();
				giantint sub_result;
				for (size_t j = 0; j < i; j++)
					sub_result.data.push_back(0);
				t_dword carry = 0;
				for (; it != end; it++) {
					t_qword a = (t_qword)*it;
					t_qword b = (t_qword)other.data[i];
					t_qword r = a*b + (t_qword)carry;
					carry = (t_dword)(r >> 32);
					sub_result.data.push_back((t_dword)r);
				}
				if (carry != 0) sub_result.data.push_back(carry);
				result = result + sub_result;
			}
			result._M_signal = _M_signal*other._M_signal;
		}
		result.normalize();
		return result;
	}
	giantint do_division(const giantint& divisor, giantint& remainder) const {
		t_dword shift = 0;
		if (divisor.is_zero()) {
			shift = 1/shift;
		}
		giantint dvd(*this);
		giantint div(divisor);
		dvd._M_signal = 1;
		div._M_signal = 1;
		int cmp = dvd.compare_to(div);
		if (cmp < 0) {
			remainder = *this;
			giantint quo;
			quo._M_signal = _M_signal*divisor._M_signal;
			return quo;
		} else if (cmp == 0) {
			giantint quo(1);
			quo._M_signal = _M_signal*divisor._M_signal;
			remainder.data.clear();
			remainder._M_signal = 1;
			return quo;
		}
		if (divisor.data.size() == 1) {
			giantint quo(*this);
			quo.single_div(divisor.data[0]);
			remainder.data.push_back(single_mod(divisor.data[0]));
			quo._M_signal = _M_signal*divisor._M_signal;
			return quo;
		}
		giantint quo;
		if (div.data.size() < dvd.data.size()) {
			shift = (dvd.data.size() - div.data.size())*32;
			div <<= shift;
		}
		while (div < dvd) {
			div <<= 1;
			shift++;
		}
		if (div > dvd) {
			div >>= 1;
		}
		while (div > dvd) {
			div >>= 1;
			shift--;
		}
		for (; shift; shift--) {
			quo <<= 1;
			if (div <= dvd) {
				quo |= 1;
				dvd -= div;
			}
			div >>= 1;
		}
		remainder = dvd;
		quo._M_signal = _M_signal*divisor._M_signal;
		return quo;
	}
public:
	static giantint& zero() {
		static giantint _zero;
		return _zero;
	}
	giantint(): data(0) {
		_M_signal = 1;
	}

	giantint(t_int i): data() {
		data.push_back(::abs(i));
		_M_signal = i < 0 ? -1 : 1;
	}

	giantint(t_bigint bi): data() {
		data.push_back((t_dword)(::llabs(bi) & 0xFFFFFFFF));
		data.push_back((t_dword)(::llabs(bi) >> 32));
		_M_signal = bi < 0 ? -1 : 1;
	}

	giantint(std::string bi): data() {
		t_int i = 0;
		_M_signal = 1;
		if (bi.at(i) == '-') {
			_M_signal = -1;
			i++;
		} else if (bi.at(i) == '+') {
			i++;
		}
		for (size_t j = i; j < bi.length(); j++) {
			char ch = bi.at(j);
			if ('9' >= ch && ch >= '0') {
				single_mul(10);
				single_add(ch - '0');
			} else {
				throw std::exception();
			}
		}
	}

	giantint(std::string bi, t_dword radix): data() {
		switch (radix) {
		case 2:
		case 8:
		case 10:
		case 16:
			break;
		default:
			throw math_exception("Invalid radix value");
		}
		t_int i = 0;
		_M_signal = 1;
		if (bi.at(i) == '-') {
			_M_signal = -1;
			i++;
		} else if (bi.at(i) == '+') {
			i++;
		}
		for (size_t j = i; j < bi.length(); j++) {
			char ch = bi.at(j);
			if (('9' >= ch && ch >= '0') || (radix == 16 && (
					('a' <= ch && ch <= 'f') ||('A' <= ch && ch <= 'F')))) {
				single_mul(radix);
				if (('9' >= ch && ch >= '0'))
					single_add(ch - '0');
				if (('F' >= ch && ch >= 'A'))
					single_add(ch - 'A' + 0x0A);
				if (('f' >= ch && ch >= 'a'))
					single_add(ch - 'a' + 0x0A);
			} else {
				throw std::exception();
			}
		}
	}

	giantint(const giantint& other):
		data(other.data),
		_M_signal(other._M_signal)
	{
	}

	giantint minus() const {
		giantint result = *this;
		result._M_signal = -result._M_signal;
		return result;
	}

	t_int signal() const {
		return _M_signal;
	}

	giantint add(const giantint& other) const {
		if (_M_signal == other._M_signal) {
			return do_add(other);
		} else {
			return do_subtract(other);
		}
	}

	giantint subtract(const giantint& other) const {
		if (_M_signal == other._M_signal) {
			return do_subtract(other);
		} else {
			return do_add(other);
		}
	}

	giantint multiply(const giantint& other) const {
		return do_multiply(other);
	}

	giantint divide(const giantint& other) const {
		giantint rem;
		return do_division(other,rem);
	}

	giantint remainder(const giantint& other) const {
		giantint rem;
		do_division(other,rem);
		return rem;
	}

	giantint divide_and_remainder(const giantint& divisor, giantint& remainder) const {
		return do_division(divisor,remainder);
	}

	giantint left_shift(const t_dword l) const {
		giantint result(*this);
		return result.do_lshift(l);
	}

	giantint right_shift(const t_dword l) const {
		giantint result(*this);
		return result.do_rshift(l);
	}

	giantint bitwise_or(const t_dword l) const {
		giantint result(*this);
		return result.do_or(l);
	}

	giantint bitwise_or(const giantint& other) const {
		giantint result(*this);
		return result.do_or(other);
	}

	giantint bitwise_and(const t_dword l) const {
		giantint result(*this);
		return result.do_and(l);
	}

	giantint bitwise_and(const giantint& other) const {
		giantint result(*this);
		return result.do_and(other);
	}

	giantint bitwise_xor(const t_dword l) const {
		giantint result(*this);
		return result.do_xor(l);
	}

	giantint bitwise_xor(const giantint& other) const {
		giantint result(*this);
		return result.do_xor(other);
	}

	giantint bitwise_not() const {
		giantint result(*this);
		return result.do_not();
	}

	giantint sqrt() const {
		giantint min(1);
		giantint max(*this);
		giantint mid((max+min).single_div(2));
		while (min != max && min != max-giantint(1)) {
			giantint mid2 = mid*mid;
			if (mid2 < *this)
				min = mid;
			else if (mid2 > *this)
				max = mid;
			else return mid;
			mid = (max+min).single_div(2);
		}
		return min;
	}

	giantint pow(t_int p) const {
		if (p==0)
			return giantint(1);
		if (p<0)
			return giantint();
		giantint r = *this;
		p--;
		for (;p!=0;p--)
			r *= *this;
		return r;
	}

	bool is_zero() const {
		if (data.size() == 0) return true;
		array_type::const_iterator it = data.begin();
		array_type::const_iterator end = data.end();
		for (; it != end; it++) {
			if (*it != 0) return false;
		}
		return true;
	}

	t_int digits() const {
		t_int r = 0;
		giantint gi(*this);
		while (!gi.is_zero()) {
			gi.single_div(10);
			r++;
		}
		return r;
	}

	bool equal_to(const giantint& other) const {
		if (is_zero() && other.is_zero()) return true;
		if (_M_signal != other._M_signal) return false;
		t_size length = std::min(data.size(), other.data.size());
		for (t_size i = 0; i < length; i++) {
			if (data[i] != other.data[i]) return false;
		}
		if (data.size() == other.data.size()) return true;
		if (data.size() > other.data.size()) {
			for (t_size i = length; i < data.size(); i++) {
				if (data[i] != 0) return false;
			}
		} else {
			for (t_size i = length; i < other.data.size(); i++) {
				if (other.data[i] != 0) return false;
			}
		}
		return true;
	}

	t_int compare_to(const giantint& other) const {
		int length = (int)(std::min(data.size(), other.data.size()));
		if (data.size() > other.data.size()) {
			for (int i = (int)data.size() -1; i >= length; i--) {
				if (data[i] != 0) return _M_signal;
			}
		} else if (data.size() < other.data.size()) {
			for (int i = (int)other.data.size() -1; i >= length; i--) {
				if (other.data[i] != 0) return -_M_signal;
			}
		}
		for (int i = length -1; i >= 0; i--) {
			if (other.data[i] != data[i]) {
				if (data[i] > other.data[i])
					return _M_signal;
				else
					return -_M_signal;
			}
		}
		return _M_signal == other._M_signal ? 0 : _M_signal;
	}

	bool operator > (const giantint& other) const {
		return compare_to(other) > 0;
	}

	bool operator < (const giantint& other) const {
		return compare_to(other) < 0;
	}

	bool operator >= (const giantint& other) const {
		return compare_to(other) >= 0;
	}

	bool operator <= (const giantint& other) const {
		return compare_to(other) <= 0;
	}

	bool operator == (const giantint& other) const {
		return compare_to(other) == 0;
	}

	bool operator != (const giantint& other) const {
		return compare_to(other) != 0;
	}

	giantint operator+(const giantint& other) const {
		return add(other);
	}

	giantint operator-(const giantint& other) const {
		return subtract(other);
	}

	giantint operator*(const giantint& other) const {
		return multiply(other);
	}

	giantint operator/(const giantint& other) const {
		return divide(other);
	}

	giantint operator%(const giantint& other) const {
		return remainder(other);
	}

	giantint operator<<(const t_dword l) const {
		return left_shift(l);
	}

	giantint operator>>(const t_dword l) const {
		return right_shift(l);
	}

	giantint operator|(const t_dword l) const {
		return bitwise_or(l);
	}

	giantint operator|(const giantint& other) const {
		return bitwise_or(other);
	}

	giantint operator&(const t_dword l) const {
		return bitwise_and(l);
	}

	giantint operator&(const giantint& other) const {
		return bitwise_and(other);
	}

	giantint operator^(const t_dword l) const {
		return bitwise_xor(l);
	}

	giantint operator^(const giantint& other) const {
		return bitwise_xor(other);
	}

	giantint operator -() const {
		return minus();
	}

	giantint operator +() const {
		return *this;
	}

	giantint operator ~() const {
		return bitwise_not();
	}

	giantint& operator=(const giantint& other) {
		data = other.data;
		_M_signal = other._M_signal;
		return *this;
	}

	giantint& operator+=(const giantint& other) {
		return *this = add(other);
	}

	giantint& operator-=(const giantint& other) {
		return *this = subtract(other);
	}

	giantint& operator*=(const giantint& other) {
		return *this = multiply(other);
	}

	giantint& operator/=(const giantint& other) {
		return *this = divide(other);
	}

	giantint& operator%=(const giantint& other) {
		return *this = remainder(other);
	}

	giantint& operator<<=(const t_dword l) {
		return do_lshift(l);
	}

	giantint& operator>>=(const t_dword l) {
		return do_rshift(l);
	}

	giantint& operator|=(const t_dword l) {
		return do_or(l);
	}

	giantint& operator|=(const giantint& other) {
		return do_or(other);
	}

	giantint& operator&=(const t_dword l) {
		return do_and(l);
	}

	giantint& operator&=(const giantint& other) {
		return do_and(other);
	}

	giantint& operator^=(const t_dword l) {
		return do_xor(l);
	}

	giantint& operator^=(const giantint& other) {
		return do_xor(other);
	}

	giantint& operator ++() {
		*this = *this + giantint(1);
		return *this;
	}

	giantint operator ++(int) {
		giantint other = *this;
		++*this;
		return other;
	}

	giantint& operator --() {
		*this = *this - giantint(1);
		return *this;
	}

	giantint operator --(int) {
		giantint other = *this;
		--*this;
		return other;
	}

	std::string to_string(t_dword radix, t_int& signal) const {
		char chs[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
		switch (radix) {
		case 2:
		case 8:
		case 10:
		case 16:
			break;
		default:
			throw math_exception("Invalid radix value");
		}
		signal = _M_signal;
		std::string result;
		giantint x = *this;
		while (!x.is_zero()) {
			result.append(1,chs[x.single_mod(radix)]);
			x = x.single_div(radix);
		}
		if (result.length() == 0) result = "0";
		else {
			std::string rev(result.length(),'0');
			for (int i = 0; i < result.length(); i++) {
				rev[i] = result[result.length()-i-1];
			}
			return rev;
		}
		return result;
	}

	std::string to_string() const {
		std::string result;
		if (_M_signal == -1) result.append(1,'-');
		t_int sign;
		result += to_string(10,sign);
		return result;
	}

	int to_int() const {
		return data.empty() ? 0 : data[0];
	}

	t_bigint to_bigint() const {
		t_bigint l = 0;
		array_type::const_reverse_iterator it = data.rbegin();
		array_type::const_reverse_iterator end = data.rend();
		for (; it != end; it++) {
			l <<= 32;
			l |= *it;
		}
		return l;
	}

	t_double to_double() const {
		t_double l = 0;
		array_type::const_reverse_iterator it = data.rbegin();
		array_type::const_reverse_iterator end = data.rend();
		for (; it != end; it++) {
			l *= 4294967296.0;
			l += *it;
		}
		return l;
	}

	t_float to_float() const {
		return (t_float)to_double();
	}

};

inline std::ostream& operator<<(std::ostream& os, const giantint& gi) {
	os << gi.to_string();
	return os;
}

}

}

#endif /* GIANTINT_HPP_ */
