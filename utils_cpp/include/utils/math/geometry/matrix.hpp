/*
 * matrix.hpp
 *
 *  Created on: 27/07/2014
 *      Author: Alan
 */

#ifndef MATRIX_HPP_
#define MATRIX_HPP_

#include <utils/math/math.hpp>
#include <utils/math/geometry/vectors.hpp>
#include <utils/math/geometry/matrix_utl.hpp>
#include <utils/funcs.hpp>
#include <utils/math/math_exception.hpp>
#include <type_traits>
#include <cstdarg>
#include <iostream>
#include <limits>

namespace utils {

namespace math {

template <typename _Float, size_t _Rows, size_t _Cols>
class matrix;

template <typename _Float, size_t _Rows, size_t _Cols>
class matrix_row {
public:
	typedef size_t size_type;
	typedef _Float float_type;
	typedef float_type row_type[_Cols];
	typedef _Float& reference_type;

private:
	template<typename _Float_b, size_t _Rows_b, size_t _Cols_b>
	friend class matrix;
	template<typename _Float_b, size_t _Rows_b, size_t _Cols_b>
	friend class matrix_row;

	row_type *_M_row;
	void check_range_cols(const size_type& index) const {
		if (index >= _Cols || index < 0)
			throw utils_exception("Index out of range.");
	}

public:
	matrix_row(row_type *_row) {
		_M_row = _row;
	}
	reference_type operator[](const size_type& index) {
		check_range_cols(index);
		return (*_M_row)[index];
	}
};

template <typename _Float, size_t _Rows, size_t _Cols>
class matrix {
public:
	typedef size_t size_type;
	typedef _Float float_type;
	typedef matrix_row<_Float, _Rows, _Cols> matrixrow_type;
private:
	template<typename _Float_b, size_t _Rows_b, size_t _Cols_b>
	friend class matrix;

	float_type _M_m[_Rows][_Cols];
	void check_range_rows(const size_type& index) const {
		if (index >= _Rows || index < 0)
			throw utils_exception("Index out of range.");
	}

public:
	matrix() {
		clear_it(&_M_m[0][0],float_type(0),_Rows*_Cols);
	}

	template <typename _Float_b>
	matrix(const _Float_b _m[_Rows][_Cols]) {
		copy_it(&_M_m[0][0],&_m[0][0],_Rows*_Cols);
	}

	template <typename _Float_b>
	matrix(const matrix <_Float_b,_Rows,_Cols>& m) {
		copy_it(&_M_m[0][0],&m._M_m[0][0],_Rows*_Cols);
	}

	template <typename _Float_b>
	matrix(const _Float_b& m00, const _Float_b& m01, const _Float_b& m10, const _Float_b& m11) {
		if (_Rows != 2 || _Cols != 2)
			throw math_exception("Matrix is not 2x2.");
		_M_m[0][0] = m00; _M_m[0][1] = m01;
		_M_m[1][0] = m10; _M_m[1][1] = m11;
	}

	template <typename _Float_b>
	matrix(const _Float_b& m00, const _Float_b& m01, const _Float_b& m02,
		   const _Float_b& m10, const _Float_b& m11, const _Float_b& m12,
		   const _Float_b& m20, const _Float_b& m21, const _Float_b& m22) {
		if (_Rows != 3 || _Cols != 3)
			throw math_exception("Matrix is not 3x3.");
		_M_m[0][0] = m00; _M_m[0][1] = m01; _M_m[0][2] = m02;
		_M_m[1][0] = m10; _M_m[1][1] = m11; _M_m[1][2] = m12;
		_M_m[2][0] = m20; _M_m[2][1] = m21; _M_m[2][2] = m22;
	}

	template <typename _Float_b>
	matrix(const _Float_b& m00, const _Float_b& m01, const _Float_b& m02, const _Float_b& m03,
		   const _Float_b& m10, const _Float_b& m11, const _Float_b& m12, const _Float_b& m13,
		   const _Float_b& m20, const _Float_b& m21, const _Float_b& m22, const _Float_b& m23,
		   const _Float_b& m30, const _Float_b& m31, const _Float_b& m32, const _Float_b& m33) {
		if (_Rows != 4 || _Cols != 4)
			throw math_exception("Matrix is not 4x4.");
		_M_m[0][0] = m00; _M_m[0][1] = m01; _M_m[0][2] = m02; _M_m[0][3] = m03;
		_M_m[1][0] = m10; _M_m[1][1] = m11; _M_m[1][2] = m12; _M_m[1][3] = m13;
		_M_m[2][0] = m20; _M_m[2][1] = m21; _M_m[2][2] = m22; _M_m[2][3] = m23;
		_M_m[3][0] = m30; _M_m[3][1] = m31; _M_m[3][2] = m32; _M_m[3][3] = m33;
	}

	matrix(const float_type m00, ...) {
		va_list ap;
		va_start(ap, m00);
		float_type* mp = &_M_m[0][0];
		float_type* end = mp + _Rows*_Cols;
		*mp = m00;
		mp++;
		for (; mp < end; mp++) {
			if (sizeof(long double) == sizeof(float_type))
				*mp = va_arg(ap, long double);
			else
				*mp = va_arg(ap, double);
		}
		va_end(ap);
	}

	matrixrow_type operator[] (const size_type& index) {
		check_range_rows(index);
		return matrixrow_type(&_M_m[index]);
	}

	template <typename _Float_b>
	matrix& add(const matrix<_Float_b,_Rows,_Cols>& b) {
		for (size_type i = 0; i < _Rows; i++)
			for (size_type j = 0; j < _Cols; j++)
				_M_m[i][j] += b._M_m[i][j];
		return *this;
	}

	template <typename _Float_b>
	matrix& subtract(const matrix<_Float_b,_Rows,_Cols>& b) {
		for (size_type i = 0; i < _Rows; i++)
			for (size_type j = 0; j < _Cols; j++)
				_M_m[i][j] -= b._M_m[i][j];
		return *this;
	}

	template <typename _Float_b>
	matrix& scale(const _Float_b& b) {
		for (size_type i = 0; i < _Rows; i++)
			for (size_type j = 0; j < _Cols; j++)
				_M_m[i][j] *= b;
		return *this;
	}

	template <typename _Float_b, size_t _Cols_b>
	matrix<_Float,_Rows,_Cols_b> multiply(const matrix<_Float_b,_Cols,_Cols_b>& b) const {
		matrix<_Float,_Rows,_Cols_b> r;
		for (size_type i = 0; i < _Rows; i++)
			for (size_type j = 0; j < _Cols_b; j++)
				for (size_type k = 0; k < _Cols; k++)
					r._M_m[i][j] += _M_m[i][k]*b._M_m[k][j];
		return r;
	}

	template <typename _Float_b>
	matrix& multiply_self(const matrix<_Float_b,_Cols,_Cols>& b) {
		matrix a = *this;
		for (size_type i = 0; i < _Rows; i++)
			for (size_type j = 0; j < _Cols; j++) {
				_M_m[i][j] = 0;
				for (size_type k = 0; k < _Cols; k++)
					_M_m[i][j] += a._M_m[i][k]*b._M_m[k][j];
			}
		return *this;
	}

	template <typename _Float_b>
	vector<_Float,_Rows> multiply(const vector<_Float_b,_Cols>& b) const {
		vector<_Float,_Rows> r;
		for (size_type i = 0; i < _Rows; i++)
			for (size_type k = 0; k < _Cols; k++)
				r._M_v[i] += _M_m[i][k]*b._M_v[k];
		return r;
	}

	matrix& transpose() {
		if (_Cols != _Rows)
			throw math_exception("Cannot transpose matrix.");
		for (size_type i = 0; i < _Rows; i++)
			for (size_type j = i + 1; j < _Cols; j++)
				swap(_M_m[i][j], _M_m[j][i]);
		return *this;
	}

	matrix inverse() const {
		matrix r;
		if (_Cols != _Rows)
			throw math_exception("Cannot invert matrix.");
		if (_Rows == 2)
			matrix_utl::invert_2x2<float_type,_Rows,_Cols>(r._M_m,_M_m);
		else if (_Rows == 3)
			matrix_utl::invert_3x3<float_type,_Rows,_Cols>(r._M_m,_M_m);
		else if (_Rows == 4)
			matrix_utl::invert_4x4<float_type,_Rows,_Cols>(r._M_m,_M_m);
		else {
			matrix s = *this;
			matrix_utl::invert_NxM<float_type,_Rows,_Cols>(r._M_m,s._M_m);
		}
		return r;
	}

	float_type determinant() const {
		if (_Cols != _Rows)
			throw math_exception("Cannot calculate the determinant.");
		if (_Rows == 2)
			return matrix_utl::det_2x2<float_type,_Rows,_Cols>(_M_m);
		if (_Rows == 3)
			return matrix_utl::det_3x3<float_type,_Rows,_Cols>(_M_m);
		if (_Rows == 4)
			return matrix_utl::det_4x4<float_type,_Rows,_Cols>(_M_m);
		return matrix_utl::det_NxM<float_type,_Rows,_Cols>(_M_m,_Rows);
	}

	std::string to_string() const {
		std::string r = "(";
		char buf[32];
		for (size_type i = 0; i < _Rows; i++) {
			if (i > 0)
				r += ", ";
			r += "(";
			for (size_type j = 0; j < _Cols; j++) {
				if (j > 0)
					r += ", ";
				::gcvt(_M_m[i][j], 7, buf);
				r += buf;
			}
			r += ")";
		}
		r += ")";
		return r;
	}

	const _Float* c_matrix() const {
		return &_M_m[0][0];
	}

	template <typename _Float_b>
	matrix& operator = (const matrix<_Float_b,_Rows,_Cols>& m) {
		const _Float_b* _m = &m._M_m[0][0];
		copy_it(&_M_m[0][0],_m,_Rows*_Cols);
		return *this;
	}

	template <typename _Float_b>
	matrix& operator += (const matrix<_Float_b,_Rows,_Cols>& b) {
		return add(b);
	}

	template <typename _Float_b>
	matrix& operator -= (const matrix<_Float_b,_Rows,_Cols>& b) {
		return subtract(b);
	}

	template <typename _Float_b>
	matrix& operator *= (const matrix<_Float_b,_Cols,_Cols>& b) {
		return multiply_self(b);
	}

	template <typename _Float_b>
	matrix& operator *= (const _Float_b& b) {
		return scale(b);
	}

	template <typename _Float_b>
	bool operator == (const matrix<_Float_b,_Rows,_Cols>& m) const {
		return compare_it(&_M_m[0][0],&m._M_m[0][0],_Rows*_Cols) == 0;
	}

	template <typename _Float_b>
	bool operator != (const matrix<_Float_b,_Rows,_Cols>& m) const {
		return compare_it(&_M_m[0][0],&m._M_m[0][0],_Rows*_Cols) != 0;
	}

	template <typename _Float_b>
	bool operator > (const matrix<_Float_b,_Rows,_Cols>& m) const {
		return matrix_utl::is_matrix_greater_than<_Float_b,_Rows,_Cols>(_M_m[0][0],m._M_m[0][0]);
	}

	template <typename _Float_b>
	bool operator < (const matrix<_Float_b,_Rows,_Cols>& m) const {
		return matrix_utl::is_matrix_less_than<_Float_b,_Rows,_Cols>(_M_m[0][0],m._M_m[0][0]);
	}

	template <typename _Float_b>
	bool operator <= (const matrix<_Float_b,_Rows,_Cols>& m) const {
		return !matrix_utl::is_matrix_greater_than<_Float_b,_Rows,_Cols>(_M_m[0][0],m._M_m[0][0]);
	}

	template <typename _Float_b>
	bool operator >= (const matrix<_Float_b,_Rows,_Cols>& m) const {
		return !matrix_utl::is_matrix_less_than<_Float_b,_Rows,_Cols>(_M_m[0][0],m._M_m[0][0]);
	}

	static matrix identity() {
		static _Float _identity[_Rows][_Cols];
		static bool _initialized = false;
		if (!_initialized) {
			matrix_utl::set_identy<_Float,_Rows,_Cols>(_identity);
			_initialized = true;
		}
		return matrix(_identity);
	}

	static matrix rotation(float_type x, float_type y, float_type z) {
		float_type _rot[_Rows][_Cols];
		matrix_utl::set_rotation<_Float,_Rows,_Cols>(_rot,x,y,z);
		return matrix(_rot);
	}

};

template <typename _Float_a, typename _Float_b, size_t _Rows, size_t _Cols>
matrix<_Float_a,_Rows,_Cols> operator + (const matrix<_Float_a,_Rows,_Cols>& a, const matrix<_Float_b,_Rows,_Cols>& b) {
	matrix<_Float_a,_Rows,_Cols> r = a;
	r.add(b);
	return r;
}

template <typename _Float_a, typename _Float_b, size_t _Rows, size_t _Cols>
matrix<_Float_a,_Rows,_Cols> operator - (const matrix<_Float_a,_Rows,_Cols>& a, const matrix<_Float_b,_Rows,_Cols>& b) {
	matrix<_Float_a,_Rows,_Cols> r = a;
	r.subtract(b);
	return r;
}

template <typename _Float_a, typename _Float_b, size_t _Rows, size_t _Cols>
matrix<_Float_a,_Rows,_Cols> operator * (const matrix<_Float_a,_Rows,_Cols>& a, const _Float_b& b) {
	matrix<_Float_a,_Rows,_Cols> r = a;
	r.scale(b);
	return r;
}

template <typename _Float_a, typename _Float_b, size_t _Rows, size_t _Cols, size_t _Cols_b>
matrix<_Float_a,_Rows,_Cols_b> operator * (const matrix<_Float_a,_Rows,_Cols>& a, const matrix<_Float_b,_Cols,_Cols_b>& b) {
	return a.multiply(b);
}

template <typename _Float_a, typename _Float_b, size_t _Rows, size_t _Cols>
vector<_Float_a,_Rows> operator * (const matrix<_Float_a,_Rows,_Cols>& a, const vector<_Float_b,_Cols>& b) {
	return a.multiply(b);
}

template <typename _Float, size_t _Rows, size_t _Cols>
matrix<_Float,_Rows,_Cols> operator ~ (const matrix<_Float,_Rows,_Cols>& a) {
	return a.inverse();
}

template <typename _Float, size_t _Rows, size_t _Cols>
inline std::ostream& operator<<(std::ostream& os, const matrix<_Float,_Rows,_Cols>& v) {
	os << v.to_string();
	return os;
}

} // math

} // utils


#endif /* MATRIX_HPP_ */
