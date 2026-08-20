/*
 * vectors.hpp
 *
 *  Created on: 19/07/2014
 *      Author: Alan
 */

#ifndef VECTORS_HPP_
#define VECTORS_HPP_

#include <utils/math/math.hpp>
#include <utils/funcs.hpp>
#include <type_traits>
#include <cstdarg>

namespace utils {

namespace math {

template <typename _Float, size_t _Rows, size_t _Cols>
class matrix;

template <typename _Float, size_t _Vec_size>
class vector {
public:
	typedef size_t size_type;
	typedef _Float float_type;
	// A raw pointer is a conforming random-access iterator, and is what
	// std::array exposes. Note this drops the bounds checking the old
	// containers::iterators::pointer_iterator performed on ++ and *, matching
	// standard-container behaviour: use at() where a check is wanted.
	typedef _Float* iterator;
	typedef const _Float* const_iterator;
private:
	template <typename _Float_b, size_t _Rows, size_t _Cols>
	friend class matrix;

	typedef typename std::is_floating_point<_Float> float_trait;

	void check_type() const {
		if (!float_trait::value)
			throw utils_exception("Type must be floating point.");
	}

	void check_size(size_type _size) const {
		if (_size != _Vec_size)
			throw utils_exception("Invalid size.");
	}

	void check_range(size_type index) const {
		if (index >= _Vec_size || index < 0)
			throw utils_exception("Index out of range.");
	}

	void zero_vector(float_type* _array) {
		if (_Vec_size == 1) {
			_array[0] = 0;
		} else if (_Vec_size == 2) {
			_array[0] = 0;
			_array[1] = 0;
		} else if (_Vec_size == 3) {
			_array[0] = 0;
			_array[1] = 0;
			_array[2] = 0;
		} else if (_Vec_size == 4) {
			_array[0] = 0;
			_array[1] = 0;
			_array[2] = 0;
			_array[3] = 0;
		} else {
			::memset(_array, 0, _Vec_size*sizeof(float_type));
		}
	}

	template <typename _Float_b>
	void add_sig(const vector<_Float_b, _Vec_size>& b, float_type sig) {
		if (_Vec_size == 1) {
			_M_v[0] = _M_v[0] + sig*b[0];
		} else if (_Vec_size == 2) {
			_M_v[0] = _M_v[0] + sig*b[0];
			_M_v[1] = _M_v[1] + sig*b[1];
		} else if (_Vec_size == 3) {
			_M_v[0] = _M_v[0] + sig*b[0];
			_M_v[1] = _M_v[1] + sig*b[1];
			_M_v[2] = _M_v[2] + sig*b[2];
		} else if (_Vec_size == 4) {
			_M_v[0] = _M_v[0] + sig*b[0];
			_M_v[1] = _M_v[1] + sig*b[1];
			_M_v[2] = _M_v[2] + sig*b[2];
			_M_v[3] = _M_v[3] + sig*b[3];
		} else {
			for (size_type i = 0; i < _Vec_size; i++)
				_M_v[i] = _M_v[i] + sig*b[i];
		}
	}

	static vector create_nanvector() {
		vector vec;
		for (size_type i = 0; i < _Vec_size; i++)
			vec._M_v[i] = NAN;
		return vec;
	}

	float_type _M_v[_Vec_size];

public:
	explicit
	vector() {
		check_type();
		zero_vector(_M_v);
	}

    template<typename _InputIterator>
	vector(_InputIterator __first, _InputIterator __last) {
		check_type();
		check_size(__last-__first);
		copy_it(_M_v,__first,_Vec_size);
	}

	vector(float_type first, ...) {
		va_list ap;
		va_start(ap, first);
		check_type();
		size_type i = 1;
		size_type n = _Vec_size;
		_M_v[0] = first;
		for (; i < n; i++) {
			if (sizeof(long double) == sizeof(float_type))
				_M_v[i] = va_arg(ap, long double);
			else
				_M_v[i] = va_arg(ap, double);
		}
		va_end(ap);
	}

	template <typename _Float_b>
	vector(const vector<_Float_b, _Vec_size>& b) {
		check_type();
		for (size_type i = 0; i < _Vec_size; i++)
			_M_v[i] = (float_type) b.get(i);
	}

	~vector() {
	}

	template <typename _Float_b>
	vector& add(const vector<_Float_b, _Vec_size>& b) {
		add_sig(b,1);
		return *this;
	}

	template <typename _Float_b>
	vector& subtract(const vector<_Float_b, _Vec_size>& b) {
		add_sig(b,-1);
		return *this;
	}

	vector& multiply(const float_type& b) {
		if (_Vec_size == 1) {
			_M_v[0] *= b;
		} else if (_Vec_size == 2) {
			_M_v[0] *= b;
			_M_v[1] *= b;
		} else if (_Vec_size == 3) {
			_M_v[0] *= b;
			_M_v[1] *= b;
			_M_v[2] *= b;
		} else if (_Vec_size == 4) {
			_M_v[0] *= b;
			_M_v[1] *= b;
			_M_v[2] *= b;
			_M_v[3] *= b;
		} else {
			for (size_type i = 0; i < _Vec_size; i++)
				_M_v[i] *= b;
		}
		return *this;
	}

	template <typename _Float_b>
	vector cross(const vector<_Float_b,_Vec_size>& b) const {
		if (_Vec_size < 3) {
			throw utils_exception("Invalid size.");
		}
		vector r;
		r._M_v[0] = _M_v[1]*b[2] - _M_v[2]*b[1];
		r._M_v[1] = _M_v[2]*b[0] - _M_v[0]*b[2];
		r._M_v[2] = _M_v[0]*b[1] - _M_v[1]*b[0];
		return r;
	}

	template <typename _Float_b>
	float_type dot(const vector<_Float_b, _Vec_size>& b) const {
		if (_Vec_size == 1) {
			return _M_v[0] * b[0];
		} else if (_Vec_size == 2) {
			return _M_v[0] * b[0] + _M_v[1] * b[1];
		} else if (_Vec_size == 3) {
			return _M_v[0] * b[0] + _M_v[1] * b[1] + _M_v[2] * b[2];
		} else if (_Vec_size == 4) {
			return _M_v[0] * b[0] + _M_v[1] * b[1] + _M_v[2] * b[2] + _M_v[3] * b[3];
		} else {
			float_type r = 0;
			for (size_type i = 0; i < _Vec_size; i++)
				r += _M_v[i] * b[i];
			return r;
		}
	}

	vector minus() const {
		vector r;
		r._M_v[0] = -_M_v[0];
		r._M_v[1] = -_M_v[1];
		r._M_v[2] = -_M_v[2];
		return r;
	}

	iterator begin() {
		return _M_v;
	}
	const_iterator begin() const {
		return _M_v;
	}
	const_iterator cbegin() const {
		return _M_v;
	}

	const_iterator end() const {
		return _M_v+_Vec_size;
	}
	const_iterator cend() const {
		return _M_v+_Vec_size;
	}
	iterator end() {
		return _M_v+_Vec_size;
	}

	vector plus() const {
		return *this;
	}

	float_type sqr() const {
		return dot(*this);
	}

	float_type length() const {
		return Math::sqrt(sqr());
	}

	float_type get(const size_type& index) const {
		check_range(index);
		return _M_v[index];
	}

	void set(const size_type& index, const float_type& value) {
		check_range(index);
		_M_v[index] = value;
	}

	size_type size() const {
		return _Vec_size;
	}

	template <typename _Float_b>
	vector& operator += (const vector<_Float_b, _Vec_size>& b) {
		return add(b);
	}

	template <typename _Float_b>
	vector& operator -= (const vector<_Float_b, _Vec_size>& b) {
		return subtract(b);
	}

	vector operator - () const {
		return minus();
	}

	vector operator + () const {
		return plus();
	}

	vector& operator *= (const float_type& b) {
		return multiply(b);
	}

	vector operator * (const float_type& b) {
		vector r = *this;
		return r.multiply(b);
	}

	template <typename _Float_b>
	float_type operator * (const vector<_Float_b,_Vec_size>& b) const {
		return dot(b);
	}

	template <typename _Float_b>
	vector operator & (const vector<_Float_b,_Vec_size>& b) const {
		return cross(b);
	}

	float_type& operator [] (const size_type& index) {
		check_range(index);
		return _M_v[index];
	}

	const float_type& operator [] (const size_type& index) const {
		check_range(index);
		return _M_v[index];
	}

	template <typename _Float_b>
	bool operator == (const vector<_Float_b,_Vec_size>& b) const {
		if (_Vec_size == 1) {
			return _M_v[0] == b[0];
		} else if (_Vec_size == 2) {
			return _M_v[0] == b[0] && _M_v[1] == b[1];
		} else if (_Vec_size == 3) {
			return _M_v[0] == b[0] && _M_v[1] == b[1] && _M_v[2] == b[2];
		} else if (_Vec_size == 4) {
			return _M_v[0] == b[0] && _M_v[1] == b[1] && _M_v[2] == b[2] && _M_v[3] == b[3];
		} else {
			for (size_type i = 0; i < _Vec_size; i++)
				if (_M_v[i] != b._M_v[i]) return false;
			return true;
		}
	}

	template <typename _Float_b>
	bool operator != (const vector<_Float_b,_Vec_size>& b) const {
		return !(*this == b);
	}

	template <typename _Float_b>
	bool operator > (const vector<_Float_b,_Vec_size>& b) const {
		return sqr() > b.sqr();
	}

	template <typename _Float_b>
	bool operator < (const vector<_Float_b,_Vec_size>& b) const {
		return sqr() < b.sqr();
	}

	template <typename _Float_b>
	bool operator >= (const vector<_Float_b,_Vec_size>& b) const {
		return sqr() >= b.sqr();
	}

	template <typename _Float_b>
	bool operator <= (const vector<_Float_b,_Vec_size>& b) const {
		return sqr() <= b.sqr();
	}

	std::string to_string() const {
		std::string r = "(";
		char buf[32];
		for (size_type i = 0; i < _Vec_size; i++) {
			if (i > 0)
				r += ", ";
			::gcvt(_M_v[i], 7, buf);
			r += buf;
		}
		r += ")";
		return r;
	}

	static vector NaN_vector() {
		static vector nan_vector = create_nanvector();
		return nan_vector;
	}

};

template <typename _Float_a, typename _Float_b, size_t _Vec_size>
inline vector<_Float_a,_Vec_size> operator + (const vector<_Float_a,_Vec_size>& a, const vector<_Float_b,_Vec_size>& b) {
	vector<_Float_a,_Vec_size> r = a;
	r += b;
	return r;
}

template <typename _Float_a, typename _Float_b, size_t _Vec_size>
inline vector<_Float_a,_Vec_size> operator - (const vector<_Float_a,_Vec_size>& a, const vector<_Float_b,_Vec_size>& b) {
	vector<_Float_a,_Vec_size> r = a;
	r -= b;
	return r;
}

template <typename _Float_a, typename _Float_b, size_t _Vec_size>
inline vector<_Float_a,_Vec_size> operator * (const vector<_Float_a,_Vec_size>& a, const _Float_b& b) {
	vector<_Float_a,_Vec_size> r = a;
	r *= b;
	return r;
}

template <typename _Float_a, typename _Float_b, size_t _Vec_size>
inline _Float_a operator * (const vector<_Float_a,_Vec_size>& a, const vector<_Float_b,_Vec_size>& b) {
	return a.dot(b);
}

template <typename _Float_a, typename _Float_b, size_t _Vec_size>
inline vector<_Float_a,_Vec_size> operator & (const vector<_Float_a,_Vec_size>& a, const vector<_Float_b,_Vec_size>& b) {
	return a.cross(b);
}

template <typename _Float, size_t _Vec_size>
inline std::ostream& operator<<(std::ostream& os, const vector<_Float,_Vec_size>& v) {
	os << v.to_string();
	return os;
}

} // math

} // utils

#endif /* VECTORS_HPP_ */
