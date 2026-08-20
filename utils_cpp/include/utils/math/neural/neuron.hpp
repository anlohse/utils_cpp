/*
 * neuron.h
 *
 *  Created on: 15/07/2015
 *      Author: Alan
 */

#ifndef INCLUDE_UTILS_MATH_NEURAL_NEURON_HPP_
#define INCLUDE_UTILS_MATH_NEURAL_NEURON_HPP_

#include <utils/utils_defs.hpp>
#include <utils/funcs.hpp>
#include <utils/math/math.hpp>
#include <utils/math/statictics.hpp>
#include <memory>
#include <vector>
#include <vector>

namespace utils {
namespace math {

template <typename _Float>
struct transfer_function {

	_Float operator()(_Float x) const {
		return x;
	}

	_Float derivative(_Float x) const {
		return 1;
	}

	_Float derivative_y(_Float y) const {
		return 1;
	}

};

template <typename _Float>
struct tanh_transfer_function {

	_Float operator()(_Float x) const {
		return Math::tanh(x);
	}

	_Float derivative(_Float x) const {
		_Float y = Math::tanh(x);
		return derivative_y(y);
	}

	_Float derivative_y(_Float y) const {
		return 1 - y*y;
	}

};

template <typename _Float>
struct sigmoid_transfer_function {

	_Float operator()(_Float x) const {
		return 1/(1-Math::exp(-x));
	}

	_Float derivative(_Float x) const {
		_Float y = operator()(x);
		return derivative_y(y);
	}

	_Float derivative_y(_Float y) const {
		return y*(1 - y);
	}

};

template <typename _Float>
struct threshold_transfer_function {

	_Float operator()(_Float x) const {
		return x < -1 ? -1 : (x > 1 ? 1 : x);
	}

	_Float derivative(_Float x) const {
		_Float y = operator()(x);
		return derivative_y(y);
	}

	_Float derivative_y(_Float y) const {
		return y > -1 && y < 1 ? 1 : 0;
	}

};

template <typename _Float>
struct null_training_data {

	void set_gradient(_Float g) { }

	_Float get_gradient() const { return 0; }

};

template <typename _Float>
struct default_training_data {

	_Float gradient;

	default_training_data() : gradient(0) { }

	void set_gradient(_Float g) { gradient = g; }

	_Float get_gradient() const { return gradient; }

};

template <typename _Float, class _TransferFunc = tanh_transfer_function<_Float>, class _Alloc = DEFAULT_ALLOCATOR<_Float>, class _TrainingData = null_training_data<_Float> >
class perceptron_trainer;

template <typename _Float, class _TransferFunc = tanh_transfer_function<_Float>, class _Alloc = DEFAULT_ALLOCATOR<_Float>, class _TrainingData = null_training_data<_Float> >
class neuron {
public:
	typedef _Float float_type;
	typedef size_t size_type;
	typedef _Alloc allocator_type;
	typedef _TransferFunc transfer_function_type;
	typedef std::vector<float_type,allocator_type> weights_type;
	typedef typename weights_type::iterator weight_iterator;
	typedef typename weights_type::const_iterator const_weight_iterator;
	typedef _TrainingData training_data_type;

protected:

	friend class perceptron_trainer<_Float, _TransferFunc, _Alloc, _TrainingData>;

	weights_type _weights;
	transfer_function_type _transferFunc;
	training_data_type _training_data;
	float_type _result;

public:
	neuron(size_type count = 0) : _weights(count,float_type(0)), _transferFunc(), _training_data(), _result(0) {
	}
	neuron(const neuron& other) : _weights(other._weights), _transferFunc(other._transferFunc), _training_data(other._training_data), _result(other._result) {
	}
	~neuron() {
	}

	template <class _FloatRandom>
	void randomize(_FloatRandom& _rand) {
		weight_iterator it = _weights.begin();
		weight_iterator end = _weights.end();
		for (; it < end; ++it)
			*it = _rand()*2 - 1;
	}

	template <class _Iterator>
	float_type evaluate(_Iterator begin, _Iterator end) {
		const_weight_iterator it = _weights.begin();
		const_weight_iterator _end = _weights.end();
		_Iterator itd = begin;
		float_type v = 0;
		for (; it < _end && itd < end; ++it, ++itd)
			v += (*it)*(*itd);
		_result = _transferFunc(v);
		return _result;
	}

	training_data_type& training_data() {
		return _training_data;
	}

	transfer_function_type& transfer_function() {
		return _transferFunc;
	}

	weight_iterator begin_weight() {
		return _weights.begin();
	}

	weight_iterator end_weight() {
		return _weights.end();
	}

	const_weight_iterator begin_weight() const {
		return _weights.begin();
	}

	const_weight_iterator end_weight() const {
		return _weights.end();
	}

	float_type get_result() const {
		return _result;
	}

	operator float_type () const {
		return _result;
	}

};

template <typename _Float, class _TransferFunc = tanh_transfer_function<_Float>, class _Alloc = DEFAULT_ALLOCATOR<_Float>, class _TrainingData = null_training_data<_Float> >
class layer {
public:
	typedef neuron<_Float, _TransferFunc, _Alloc, _TrainingData> neuron_type;
	typedef _Float float_type;
	typedef size_t size_type;
	typedef _Alloc allocator_type;
	// _Alloc::rebind was removed from std::allocator in C++20.
	typedef typename std::allocator_traits<_Alloc>::template rebind_alloc<neuron_type> neuron_allocator_type;
	typedef _TransferFunc transfer_function_type;
	typedef std::vector<neuron_type,neuron_allocator_type> neuron_array_type;
	typedef typename neuron_array_type::iterator neuron_iterator;
	typedef typename neuron_array_type::const_iterator const_neuron_iterator;
	typedef _TrainingData training_data_type;

private:
	neuron_array_type _neurons;

	friend class perceptron_trainer<_Float, _TransferFunc, _Alloc, _TrainingData>;

public:

	layer(size_type size, size_type synapse_count) : _neurons(size,neuron_type(synapse_count)) {
	}

	~layer() {
	}

	template <class _FloatRandom>
	void randomize(_FloatRandom& _rand) {
		neuron_iterator it = _neurons.begin();
		neuron_iterator end = _neurons.end();
		for (; it < end; ++it)
			it->randomize(_rand);
	}

	template <class _InputIterator, class _OutputIterator>
	void evaluate(_InputIterator begin, _InputIterator end, _OutputIterator res_begin, _OutputIterator res_end) {
		neuron_iterator it = _neurons.begin();
		neuron_iterator _end = _neurons.end();
		_OutputIterator ito = res_begin;
		float_type v = 0;
		for (; it < _end && ito < res_end; ++it, ++ito)
			*ito = it->evaluate(begin,end);
	}

	template <class _InputIterator>
	void evaluate(_InputIterator begin, _InputIterator end) {
		neuron_iterator it = _neurons.begin();
		neuron_iterator _end = _neurons.end();
		float_type v = 0;
		for (; it < _end; ++it)
			it->evaluate(begin,end);
	}

	neuron_iterator begin() {
		return _neurons.begin();
	}

	neuron_iterator end() {
		return _neurons.end();
	}

	const_neuron_iterator begin() const {
		return _neurons.begin();
	}

	const_neuron_iterator end() const {
		return _neurons.end();
	}

};

template <typename _Float, class _TransferFunc = tanh_transfer_function<_Float>, class _Alloc = DEFAULT_ALLOCATOR<_Float>, class _TrainingData = null_training_data<_Float> >
class neural_network {
public:
	typedef layer<_Float, _TransferFunc, _Alloc, _TrainingData> layer_type;
	typedef _Float float_type;
	typedef size_t size_type;
	typedef _Alloc allocator_type;
	typedef typename _Alloc::template rebind<layer_type>::other layer_allocator_type;
	typedef _TransferFunc transfer_function_type;
	typedef std::vector<layer_type,layer_allocator_type> layers_type;
	typedef typename layers_type::iterator layer_iterator;
	typedef typename layers_type::const_iterator const_layer_iterator;
	typedef _TrainingData training_data_type;

private:
	layers_type _layers;

	friend class perceptron_trainer<_Float, _TransferFunc, _Alloc, _TrainingData>;

	template<class _Iterator>
	void assign_topology(_Iterator topology, _Iterator topology_end) {
		_Iterator it = topology + 1;
		for (; it < topology_end; ++it) {
			size_type size = *it;
			size_type synapse_count = *(it - 1);
			_layers.push_back(layer_type(size,synapse_count));
		}
	}

public:
	neural_network() : _layers() {
	}

	template<class _Iterator>
	neural_network(_Iterator topology, _Iterator topology_end) : _layers() {
		assign_topology(topology, topology_end);
	}

	template<size_t _N, typename _IntType>
	neural_network(const _IntType (& array)[_N]) : _layers() {
		assign_topology(array,array+_N);
	}

	~neural_network() {
	}

	template <class _FloatRandom>
	void randomize(_FloatRandom& _rand) {
		layer_iterator it = _layers.begin();
		layer_iterator end = _layers.end();
		for (; it < end; ++it)
			it->randomize(_rand);
	}

	template <class _InputIterator, class _OutputIterator>
	void evaluate(_InputIterator begin, _InputIterator end, _OutputIterator res_begin, _OutputIterator res_end) {
		layer_iterator it = _layers.begin();
		layer_iterator _end = _layers.end();
		it->evaluate(begin, end);
		++it;
		for (; it < _end; ++it)
			it->evaluate((it-1)->begin(), (it-1)->end());
		layer_type& last = _layers.back();
		_OutputIterator ito = res_begin;
		typename layer_type::const_neuron_iterator itn = last.begin();
		typename layer_type::const_neuron_iterator endn = last.end();
		for (; ito < res_end && itn < endn; ++ito, ++itn)
			*ito = *itn;
	}

	layer_iterator begin() {
		return _layers.begin();
	}

	layer_iterator end() {
		return _layers.end();
	}

	const_layer_iterator begin() const {
		return _layers.begin();
	}

	const_layer_iterator end() const {
		return _layers.end();
	}

};


template <typename _Float, class _TransferFunc, class _Alloc, class _TrainingData>
class perceptron_trainer {
public:
	typedef neural_network<_Float, _TransferFunc, _Alloc, _TrainingData> network_type;
	typedef _Float float_type;
	typedef size_t size_type;

private:
	float_type _avg_error, _error, _eta, _alpha;
	size_type _iteration;

	template<class _Iterator>
	void _calc_output_gradient(network_type& net, _Iterator target_begin, _Iterator target_end) {
		typename network_type::layer_type& outputLayer = net._layers.back();
		typename network_type::layer_type::neuron_iterator it = outputLayer.begin();
		typename network_type::layer_type::neuron_iterator end = outputLayer.end();
		_Iterator itt = target_begin;
		for (; it < end && itt < target_end; ++it, ++itt) {
			float_type d = *itt - it->get_result();
			it->training_data().set_gradient(d * it->transfer_function().derivative_y(it->get_result()));
		}
	}
	void _calc_hidden_gradient(typename network_type::layer_type& actual, typename network_type::layer_type& next) {
		for (size_t i = 0; i < actual._neurons.size(); ++i) {
			float_type sum = 0;
			for (size_t j = 0; j < next._neurons.size(); ++j) {
				sum += next._neurons[j]._weights[i] * next._neurons[j].training_data().get_gradient();
			}
			actual._neurons[i].training_data().set_gradient(sum * actual._neurons[i]
				.transfer_function().derivative_y(actual._neurons[i].get_result()));
		}
	}
	void _calc_hidden_gradient(network_type& net) {
		for (size_type i = net._layers.size() - 1; i > 0; --i)
			_calc_hidden_gradient(net._layers[i-1],net._layers[i]);
	}
/*
	template<size_t _Ni>
	void _calc_input_gradient(network_type& net, float_type (& in_grad)[_Ni], float_type (& input)[_Ni]) {
		typename network_type::layer_type& fst = net._layers.front();
		_TransferFunc func;
		for (size_t i = 0; i < _Ni; ++i) {
			float_type sum = 0;
			for (size_t j = 0; j < fst._neurons.size(); ++j) {
				sum += fst._neurons[j]._weights[i] * fst._neurons[j].training_data().get_gradient();
			}
			in_grad[i] = (sum * func.derivative_y(input[i]));
		}
	}
*/
	template<size_t _Ni>
	void _update_input_weights(network_type& net, float_type (& input)[_Ni]) {
		typename network_type::layer_type& fst = net._layers.front();
		for (size_t j = 0; j < fst._neurons.size(); ++j) {
			for (size_t i = 0; i < _Ni; ++i) {
				float_type delta = fst._neurons[j].training_data().get_gradient() * input[i];
				fst._neurons[j]._weights[i] = fst._neurons[j]._weights[i] * _alpha + _eta * delta;
			}
		}
	}
	void _update_input_weights(network_type& net) {
		for (size_t k = 1; k < net._layers.size(); ++k) {
			typename network_type::layer_type& actual = net._layers[k];
			typename network_type::layer_type& prev = net._layers[k];
			for (size_t j = 1; j < actual._neurons.size(); ++j) {
				for (size_t i = 0; i < actual._neurons[j]._weights.size(); ++i) {
					float_type delta = actual._neurons[j].training_data().get_gradient() * prev._neurons[i].get_result();
					actual._neurons[j]._weights[i] = actual._neurons[j]._weights[i] * _alpha + _eta * delta;
				}
			}
		}
	}
public:
	perceptron_trainer() : _avg_error(0), _error(0), _eta(0.3), _alpha(1), _iteration(0) {
	}

	perceptron_trainer(const float_type& eta, const float_type& alpha) : _avg_error(0), _error(0),
			_eta(eta), _alpha(alpha), _iteration(0) {
	}

	template<size_t _Ni, size_t _No>
	void train(network_type& net, float_type (& input)[_Ni], float_type (& target)[_No]) {
		float_type output[_No];
		//float_type in_grad[_Ni];
		net.evaluate(input,input+_Ni,output,output+_No);
		error_measure<float_type> em;
		em.push(output,output+_No,target,target+_No);
		_error = em.rms();
		_avg_error = (_avg_error * _iteration + _error) / (_iteration + 1);
		++_iteration;
		_calc_output_gradient(net, target, target + _No);
		_calc_hidden_gradient(net);
		//_calc_input_gradient(net,in_grad,input);
		_update_input_weights(net,input);
		_update_input_weights(net);
	}

	float_type avg_error() const {
		return _avg_error;
	}
	float_type last_error() const {
		return _error;
	}
	size_type iteration() const {
		return _iteration;
	}

};


} /* namespace math */
} /* namespace utils */

#endif /* INCLUDE_UTILS_MATH_NEURAL_NEURON_HPP_ */
