/*
 * fbool.h
 *
 *  Created on: 13/07/2015
 *      Author: Alan
 */

#ifndef INCLUDE_UTILS_MATH_FUZZY_FBOOL_HPP_
#define INCLUDE_UTILS_MATH_FUZZY_FBOOL_HPP_

#include <utils/utils_defs.hpp>
#include <algorithm>
#include <utils/funcs.hpp>

namespace utils {
namespace math {

template<typename _Float>
class fbool {
public:
	typedef _Float float_type;
private:
	float_type _state;
public:
	fbool(float_type state = 0) : _state(state) { }
	fbool(const fbool& other) : _state(other._state) { }

	fbool& operator = (const fbool& other) {
		_state = other._state;
		return *this;
	}

	fbool& operator = (float_type state) {
		_state = state;
		return *this;
	}

	fbool operator && (const fbool& other) const {
		return fbool(std::min(_state, other._state));
	}

	fbool operator && (float_type other) const {
		return fbool(std::min(_state, other));
	}

	fbool operator || (const fbool& other) const {
		return fbool(std::max(_state, other._state));
	}

	fbool operator || (float_type other) const {
		return fbool(std::max(_state, other));
	}

	fbool operator ! () const {
		return fbool(1 - _state);
	}

	fbool operator ^ (const fbool& other) const {
		return (*this || other) && !(*this && other);
	}

	fbool operator ^ (float_type other) const {
		return (*this || other) && !(*this && other);
	}

};

} /* namespace math */
} /* namespace utils */

#endif /* INCLUDE_UTILS_MATH_FUZZY_FBOOL_HPP_ */
