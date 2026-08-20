/*
 * error.hpp
 *
 *  Created on: 14/08/2014
 *      Author: Alan
 */

#ifndef ERROR_HPP_
#define ERROR_HPP_

#include <utils/math/math.hpp>
#include <utils/math/math_exception.hpp>
#include <iostream>

namespace utils {

namespace math {

struct error_measure {

	static double double_min_alg;
	static double float_min_alg;
	static double long_double_min_alg;

	template<typename _Float>
	static _Float get_error(_Float n) {
		return n;
	}

};

template<>
inline double error_measure::get_error<double>(double n) {
	return n * double_min_alg;
}

template<>
inline float error_measure::get_error<float>(float n) {
	return n * float_min_alg;
}

template<>
inline long double error_measure::get_error<long double>(long double n) {
	return n * long_double_min_alg;
}


template<typename _Float>
struct errored_number {

	typedef _Float float_type;
	typedef _Float& reference_type;
	typedef _Float* pointer_type;

	float_type value;
	float_type error;

	errored_number(const float_type& n) : value(n), error(error_measure::get_error(n)) {
	}

	errored_number(const float_type& n, const float_type& e) : value(n), error(e) {
	}

	errored_number(const errored_number& b) : value(b.value), error(b.error) {
	}

	errored_number& operator = (const errored_number& b) {
		value = b.value;
		error = b.error;
		return *this;
	}

	errored_number& operator += (const errored_number& b) {
		value += b.value;
		error += b.error;
		return *this;
	}

	errored_number& operator ++ () {
		value ++;
		return *this;
	}

	errored_number& operator -- () {
		value --;
		return *this;
	}

	errored_number operator ++ (int) {
		errored_number tmp = *this;
		value ++;
		return tmp;
	}

	errored_number operator -- (int) {
		errored_number tmp = *this;
		value --;
		return tmp;
	}

	errored_number& operator -= (const errored_number& b) {
		value -= b.value;
		error += b.error;
		return *this;
	}

	errored_number& operator *= (const errored_number& b) {
		value *= b.value;
		error = Math::abs(value*b.error) + Math::abs(b.value*error);
		return *this;
	}

	errored_number& operator /= (const errored_number& b) {
		value /= b.value;
		error = Math::abs(value*b.error) + Math::abs(b.value*error);
		return *this;
	}

	reference_type operator * () {
		return value;
	}

	pointer_type operator & () {
		return &value;
	}

	bool operator == (const errored_number& b) const {
		return Math::abs(value - b.value) <= error + b.error;
	}

	bool operator != (const errored_number& b) const {
		return Math::abs(value - b.value) > error + b.error;
	}

	bool operator > (const errored_number& b) const {
		return value - error > b.value + b.error;
	}

	bool operator < (const errored_number& b) const {
		return value + error < b.value - b.error;
	}

	bool operator >= (const errored_number& b) const {
		return value + error >= b.value - b.error;
	}

	bool operator <= (const errored_number& b) const {
		return value - error <= b.value + b.error;
	}

};

template<typename _Float>
inline errored_number<_Float> operator + (const errored_number<_Float>& a, const errored_number<_Float>& b) {
	errored_number<_Float> r = a;
	return r += b;
}

template<typename _Float>
inline errored_number<_Float> operator - (const errored_number<_Float>& a, const errored_number<_Float>& b) {
	errored_number<_Float> r = a;
	return r -= b;
}

template<typename _Float>
inline errored_number<_Float> operator - (const errored_number<_Float>& a) {
	return errored_number<_Float>(-a.value,a.error);
}

template<typename _Float>
inline errored_number<_Float> operator + (const errored_number<_Float>& a) {
	return a;
}

template<typename _Float>
inline errored_number<_Float> operator * (const errored_number<_Float>& a, const errored_number<_Float>& b) {
	errored_number<_Float> r = a;
	return r *= b;
}

template<typename _Float>
inline errored_number<_Float> operator / (const errored_number<_Float>& a, const errored_number<_Float>& b) {
	errored_number<_Float> r = a;
	return r /= b;
}

template<>
inline errored_number<double> Math::ceil(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::floor(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::abs(const errored_number<double>& x) { return x < 0 ? -x : x; }

template<>
inline errored_number<double> Math::cos(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::sin(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::tan(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::acos(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::asin(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::atan(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::atan2(const errored_number<double>& x, const errored_number<double>& y) { return x; }

template<>
inline errored_number<double> Math::cosh(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::sinh(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::tanh(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::acosh(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::asinh(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::atanh(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::exp(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::ln(const errored_number<double>& x) { return x; }

template<>
inline errored_number<double> Math::sqr(const errored_number<double>& x) { return x*x; }

template<>
inline errored_number<double> Math::sqrt(const errored_number<double>& x) { return x; }

template <typename _Float>
inline std::ostream& operator<<(std::ostream& os, const errored_number<_Float>& v) {
	char buf[32];
	buf[0] = 0;
	// "\xB1" is the plus-minus sign, written as an escape so this file stays
	// pure ASCII and its meaning does not depend on the source encoding.
	os << ::gcvt(v.value, 7, buf) << " \xB1 " ;
	buf[0] = 0;
	os << ::gcvt(v.error, 7, buf);
	return os;
}

} // math

} // utils

#endif /* ERROR_HPP_ */
