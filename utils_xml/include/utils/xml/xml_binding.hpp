/*
 * xml_binding.hpp
 *
 *  Created on: 2 de jul de 2018
 *      Author: alan.lohse
 */

#ifndef UTILS_XML_XML_BINDING_HPP_
#define UTILS_XML_XML_BINDING_HPP_

#include <utils/xml/xml_parser.hpp>
#include <utils/conversion.hpp>
#include <vector>
#include <string>

namespace utils {

namespace xml {

/**
 * xml_binding_attribute maps an attribute
 */
template<typename _OwnerClass>
struct xml_binding_attribute {
	typedef _OwnerClass class_type;
	const char* attribute_name;
	const char* namespace_name;

	xml_binding_attribute(const char* _attribute_name, const char* _namespace_name) :
			attribute_name(_attribute_name), namespace_name(_namespace_name) {
	}

	virtual ~xml_binding_attribute() {
	}

	virtual void get_value_as_string(const class_type* value, char* dest, size_t max_count) const = 0;

	virtual void set_value(class_type* value, const char* _str_val) = 0;

	virtual void get_value_as_string(const class_type* value, wchar_t* dest, size_t max_count) const = 0;

	virtual void set_value(class_type* value, const wchar_t* _str_val) = 0;
};

/**
 * xml_binding_element maps an element
 */
template<typename _OwnerClass, typename _ElemClass>
struct xml_binding_element {
	typedef _OwnerClass class_type;
	typedef _ElemClass element_type;
	const char* element_name;
	const char* namespace_name;

	xml_binding_element(const char* _element_name, const char* _namespace_name) :
		element_name(_element_name), namespace_name(_namespace_name) {
	}

	virtual ~xml_binding_element() {
	}

	virtual element_type& get_value(class_type* value) const = 0;

	virtual void set_value(class_type* value, const element_type& _elem_val) = 0;

	virtual element_type create_value() = 0;
};

/**
 * xml_binding_element_list maps a list of elements
 */
template<typename _OwnerClass, typename _ElemClass>
struct xml_binding_element_list {
	typedef _OwnerClass class_type;
	typedef _ElemClass element_type;
	const char* element_name;
	const char* namespace_name;

	xml_binding_element_list(const char* _element_name, const char* _namespace_name) :
		element_name(_element_name), namespace_name(_namespace_name) {
	}

	virtual ~xml_binding_element_list() {
	}

	virtual size_t get_count(class_type* value) const = 0;

	virtual element_type& get_value(class_type* value, int index) const = 0;

	virtual void set_value(class_type* value, const element_type& _elem_val, int index) = 0;

	virtual void add_value(class_type* value, const element_type& _elem_val) = 0;

	virtual element_type create_value() = 0;
};


/**
 * xml_binding_attribute_field maps a field with direct access to an attribute
 */
template<typename _InputType, typename _OwnerClass>
struct xml_binding_attribute_field : public xml_binding_attribute<_OwnerClass> {
	typedef _OwnerClass class_type;

	_InputType _OwnerClass::*field;

	xml_binding_attribute_field(const char* _attribute_name, const char* _namespace_name, _InputType _OwnerClass::*_field) :
		xml_binding_attribute<_OwnerClass>(_attribute_name, _namespace_name), field(_field) {
	}

	virtual ~xml_binding_attribute_field() {
	}

	virtual void get_value_as_string(const class_type* value, char* dest, size_t max_count) const {
		utils::to_string(value->*field,dest,max_count);
	}

	virtual void set_value(class_type* value, const char* _str_val) {
		value->*field = utils::from_string<_InputType>(_str_val);
	}

	virtual void get_value_as_string(const class_type* value, wchar_t* dest, size_t max_count) const {
		utils::to_string(value->*field,dest,max_count);
	}

	virtual void set_value(class_type* value, const wchar_t* _str_val) {
		value->*field = utils::from_string<_InputType>(_str_val);
	}

};

/**
 * xml_binding_attribute_function maps a field with getter and setter to an attribute
 */
template<typename _InputType, typename _OwnerClass>
struct xml_binding_attribute_function : public xml_binding_attribute<_OwnerClass> {
	typedef _OwnerClass class_type;

	typedef _InputType (_OwnerClass::*getter_type)() const;
	getter_type getter;

	typedef void (_OwnerClass::*setter_type)(const _InputType& val);
	setter_type setter;

	xml_binding_attribute_function(const char* _attribute_name, const char* _namespace_name, getter_type _getter, setter_type _setter) :
		xml_binding_attribute<_OwnerClass>(_attribute_name, _namespace_name), getter(_getter), setter(_setter) {
	}

	virtual ~xml_binding_attribute_function() {
	}

	virtual void get_value_as_string(const class_type* value, char* dest, size_t max_count) const {
		utils::to_string(((*value).*getter)(),dest,max_count);
	}

	virtual void set_value(class_type* value, const char* _str_val) {
		((*value).*setter)(utils::from_string<_InputType>(_str_val));
	}

	virtual void get_value_as_string(const class_type* value, wchar_t* dest, size_t max_count) const {
		utils::to_string(((*value).*getter)(),dest,max_count);
	}

	virtual void set_value(class_type* value, const wchar_t* _str_val) {
		((*value).*setter)(utils::from_string<_InputType>(_str_val));
	}

};


/**
 * xml_binding_element_field maps an element with direct access to the attribute
 */
template<typename _OwnerClass, typename _ElemClass >
struct xml_binding_element_field : public xml_binding_element<_OwnerClass, _ElemClass> {
	typedef _OwnerClass class_type;
	typedef _ElemClass element_type;

	element_type _OwnerClass::*field;

	xml_binding_element_field(const char* _element_name, const char* _namespace_name, element_type _OwnerClass::*_field) :
		xml_binding_element<_OwnerClass, _ElemClass>(_element_name, _namespace_name), field(_field) {
	}

	virtual ~xml_binding_element_field() {
	}

	virtual element_type& get_value(class_type* value) const {
		return value->*field;
	}

	virtual void set_value(class_type* value, const element_type& _elem_val) {
		value->*field = _elem_val;
	}

	virtual element_type create_value() {
		return element_type();
	}
};


/**
 * xml_binding_element_field maps an element with getter and setter to an attribute
 */
template<typename _OwnerClass, typename _ElemClass>
struct xml_binding_element_function : public xml_binding_element<_OwnerClass, _ElemClass> {
	typedef _OwnerClass class_type;
	typedef _ElemClass element_type;

	typedef element_type (_OwnerClass::*getter_type)() const;
	getter_type getter;

	typedef void (_OwnerClass::*setter_type)(const element_type& val);
	setter_type setter;

	xml_binding_element_function(const char* _element_name, const char* _namespace_name, getter_type _getter, setter_type _setter) :
		xml_binding_element<_OwnerClass, _ElemClass>(_element_name, _namespace_name), getter(_getter), setter(_setter) {
	}

	virtual ~xml_binding_element_function() {
	}

	virtual element_type& get_value(class_type* value) const {
		return ((*value).*getter)();
	}

	virtual void set_value(class_type* value, const element_type& _elem_val) {
		((*value).*setter)(_elem_val);
	}

	virtual element_type create_value() {
		return element_type();
	}
};

template<typename _ElemClass, typename _VectorClass = std::vector<_ElemClass> >
struct vector_add_func {
	void operator() (_VectorClass& vector, const _ElemClass& elem) {
		vector.push_back(elem);
	}
};

template<typename _ElemClass, typename _VectorClass = std::vector<_ElemClass> >
struct vector_count_func {
	size_t operator() (_VectorClass& vector) {
		return vector.size();
	}
};

/**
 * xml_binding_element_list maps a list of elements
 */
template<typename _OwnerClass, typename _ElemClass, typename _VectorClass,
		class _AddFunc = vector_add_func<_ElemClass,_VectorClass>,
		class _CountFunc = vector_count_func<_ElemClass,_VectorClass> >
struct xml_binding_element_list_field : public xml_binding_element_list<_OwnerClass, _ElemClass> {
	typedef _OwnerClass class_type;
	typedef _ElemClass element_type;
	typedef _VectorClass vector_type;
	typedef size_t size_t;

	vector_type _OwnerClass::*field;

	_AddFunc add_func;
	_CountFunc count_func;

	xml_binding_element_list_field(const char* _element_name, const char* _namespace_name, vector_type _OwnerClass::*_field) :
		xml_binding_element_list<_OwnerClass, _ElemClass>(_element_name, _namespace_name), field(_field) {
	}

	virtual ~xml_binding_element_list_field() {
	}

	virtual size_t get_count(class_type* value) const {
		return count_func(value);
	}

	virtual element_type& get_value(class_type* value, int index) const {
		return (value->*field)[index];
	}

	virtual void set_value(class_type* value, const element_type& _elem_val, int index) {
		(value->*field)[index] = _elem_val;
	}

	virtual void add_value(class_type* value, const element_type& _elem_val) {
		add_func(value->*field, _elem_val);
	}

	virtual element_type create_value() {
		return element_type();
	}
};

/**
 * xml_binding_element_list maps a list of elements
 */
template<typename _OwnerClass, typename _ElemClass, typename _VectorClass,
		class _AddFunc = vector_add_func<_ElemClass,_VectorClass>,
		class _CountFunc = vector_count_func<_ElemClass,_VectorClass> >
struct xml_binding_element_list_function : public xml_binding_element_list<_OwnerClass, _ElemClass> {
	typedef _OwnerClass class_type;
	typedef _ElemClass element_type;
	typedef _VectorClass vector_type;
	typedef size_t size_t;

	typedef vector_type (_OwnerClass::*getter_type)() const;
	getter_type getter;

	typedef void (_OwnerClass::*setter_type)(const vector_type& val);
	setter_type setter;


	_AddFunc add_func;
	_CountFunc count_func;

	xml_binding_element_list_function(const char* _element_name, const char* _namespace_name, getter_type _getter, setter_type _setter) :
		xml_binding_element_list<_OwnerClass, _ElemClass>(_element_name, _namespace_name), getter(_getter), setter(_setter) {
	}

	virtual ~xml_binding_element_list_function() {
	}

	virtual size_t get_count(class_type* value) const {
		return count_func(value);
	}

	virtual element_type& get_value(class_type* value, int index) const {
		return ((*value).*getter)()[index];
	}

	virtual void set_value(class_type* value, const element_type& _elem_val, int index) {
		((*value).*getter)()[index] = _elem_val;
	}

	virtual void add_value(class_type* value, const element_type& _elem_val) {
		add_func(((*value).*getter)(), _elem_val);
	}

	virtual element_type create_value() {
		return element_type();
	}
};

#define MAP_ATTRIBUTE_F(en,nmspc,InputType,Class,field) (utils::xml::xml_binding_attribute_field<InputType,Class>(en,nmspc,field))
#define MAP_ATTRIBUTE_GS(en,nmspc,InputType,Class,getter,setter) (utils::xml::xml_binding_attribute_function<InputType,Class>(en,nmspc,getter,setter))

}

}

#endif /* UTILS_XML_XML_BINDING_HPP_ */
