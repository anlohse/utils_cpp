/*
 * fft.hpp
 *
 *  Created on: 31/01/2014
 *      Author: Alan
 */

#ifndef FFT_HPP_
#define FFT_HPP_

#include <utils/math/transforms/transform.hpp>
#include <utils/math/math.hpp>
#include <vector>
#include <type_traits>

namespace utils {

namespace math {

namespace transforms {

template <typename _Float, typename _Alloc = DEFAULT_ALLOCATOR<std::complex<_Float> > >
class DiscreteFourierTransform : public Transform<_Float> {
public:
	typedef typename std::is_floating_point<_Float> float_trait;
	typedef _Float float_type;
	typedef std::complex<_Float> complex_type;
private:
	_Alloc alloc;
	complex_type* m_coeffs;
	size_t m_length;

	template<typename _Iterator>
	void do_dft(_Iterator x, _Iterator end) {
		size_t N = (end - x);
		m_coeffs = alloc.allocate(N);
		m_length = N;
		for (size_t n = 0; n < N; n++) {
			complex_type c = *x;
			float_type t = n*Math::pi(float_type(1))/N;
			complex_type eix = std::polar(float_type(1), t);
			complex_type eiy = eix;
			for (size_t i = 1; i < N; i++) {
				c += x[i]*std::conj(eiy);
				eiy *= eix;
			}
			c /= N;
			m_coeffs[n] = c;
		}
	}

	void check_type() {
		if (!float_trait::value) throw utils_exception("Type must be floating point.");
	}

public:
	DiscreteFourierTransform() : alloc(), m_coeffs(NULL), m_length(0) {
		check_type();
		m_coeffs = alloc.allocate(1);
		m_coeffs[0] = complex_type(0,0);
		m_length = 1;
	}

	DiscreteFourierTransform(float_type* coeafs, int length) : alloc() {
		check_type();
		m_coeffs = alloc.allocate(length);
		copy_it(m_coeffs,coeafs,length);
		m_length = length;
	}

	template<typename _T>
	DiscreteFourierTransform(const DiscreteFourierTransform<_T>& other) : alloc() {
		check_type();
		m_coeffs = alloc.allocate(other.m_length);
		copy_it(m_coeffs,other.m_coeffs,other.m_length);
		m_length = other.m_length;
	}

	template<typename _Iterator>
	DiscreteFourierTransform(_Iterator x, _Iterator end) : alloc(), m_coeffs(NULL), m_length(0) {
		check_type();
		if (end==x) {
			// This used to read `DiscreteFourierTransform();`, which builds and
			// discards a temporary rather than initialising this object, so an
			// empty range left m_coeffs null instead of the single zero
			// coefficient the default constructor produces.
			m_coeffs = alloc.allocate(1);
			m_coeffs[0] = complex_type(0,0);
			m_length = 1;
		} else {
			do_dft(x,end);
		}
	}

	virtual ~DiscreteFourierTransform() {
		alloc.deallocate(m_coeffs,m_length);
	}

	virtual const complex_type f(const float_type& _x) {
		float_type x = _x;
		if (x > 1)
			x -= Math::floor(x);
		else if (x < 0)
			x = Math::ceil(x) - x;
		complex_type r = *m_coeffs;
		float_type t = x*Math::pi(float_type(1));
		complex_type eix = std::polar(float_type(1), t);
		complex_type eiy = eix;
		for (size_t i = 1; i < m_length; i++) {
			r += m_coeffs[i]*eiy;
			eiy *= eix;
		}
		return r;
	}

	virtual float_type getFundamentalFrequency() {
		return float_type(0);
	}

	virtual void normalize() {
	}

	virtual void lowPass(float_type cutoff, float_type w) {
	}

	virtual void highPass(float_type cutoff, float_type w) {
	}

	virtual void bandPass(float_type center, float_type w) {
	}

	virtual size_t size() {
		return m_length;
	}

	virtual complex_type coefficient(size_t i) {
		complex_type* cs = m_coeffs + i;
		return *cs;
	}

	template<typename _T>
	DiscreteFourierTransform<_Float>& operator = (const DiscreteFourierTransform<_T>& other) {
		alloc.deallocate(m_coeffs,m_length);
		m_coeffs = alloc.allocate(other.m_length);
		copy_it(m_coeffs,other.m_coeffs,other.m_length);
		m_length = other.m_length;
	}

};


template <typename _Float, typename _Alloc = DEFAULT_ALLOCATOR<_Float> >
class FastFourierTransform : public Transform<_Float> {
public:
	typedef std::complex<_Float> complex_type;
	typedef _Float float_type;
private:
	_Alloc alloc;
	float_type* m_coeafs;
	size_t m_length;

	template<typename _Iterator>
	void doTheFFT(_Iterator x, _Iterator endx, int isign) {
		int n, mmax, m, j, istep, i;
		float_type wtemp, wr, wpr, wpi, wi, theta, tempr, tempi;

		n = endx - x;

		m_coeafs = alloc.allocate(n);
		m_length = n;
		copy_it(m_coeafs,x,n);
		float_type* data = m_coeafs;
		float_type* end = data + n;
		bitReversal(data, end);

		mmax = 2;
		while (n > mmax) {
			istep = mmax << 1;
			theta = isign * (2 * Math::pi(float_type(1)) / mmax);
			wtemp = Math::sin(0.5 * theta);
			wpr = -2.0 * wtemp * wtemp;
			wpi = Math::sin(theta);
			wr = 1.0;
			wi = 0.0;
			for (m = 1; m < mmax; m += 2) {
				for (i = m; i <= n; i += istep) {
					j = i + mmax;
					tempr = wr * data[j - 1] - wi * data[j];
					tempi = wr * data[j] + wi * data[j - 1];
					data[j - 1] = data[i - 1] - tempr;
					data[j] = data[i] - tempi;
					data[i - 1] += tempr;
					data[i] += tempi;
				}
				wr = (wtemp = wr) * wpr - wi * wpi + wr;
				wi = wi * wpr + wtemp * wpi + wi;
			}
			mmax = istep;
		}
	}

	template<typename _Iterator>
	void bitReversal(_Iterator data, _Iterator end) {
		int n = end- data;
		int m;
		int j;
		int i;
		j = 0;
		for (i = 0; i < n / 2; i += 2) {
			if (j > i) {
				swap(data[j], data[i]);
				swap(data[j + 1], data[i + 1]);
				if ((j / 2) < (n / 4)) {
					swap(data[(n - (i + 2))], data[(n - (j + 2))]);
					swap(data[(n - (i + 2)) + 1], data[(n - (j + 2)) + 1]);
				}
			}
			m = n / 2;
			while (m >= 2 && j >= m) {
				j -= m;
				m = m / 2;
			}
			j += m;
		}
	}



public:
	FastFourierTransform() : alloc(), m_coeafs(NULL), m_length(0) {
		m_coeafs = alloc.allocate(2);
		m_coeafs[0] = 0;
		m_coeafs[1] = 0;
		m_length = 2;
	}

	FastFourierTransform(float_type* coeafs, int length) : alloc() {
		m_coeafs = alloc.allocate(length);
		copy_it(m_coeafs,coeafs,length);
		m_length = length;
	}

	template<typename _Iterator>
	FastFourierTransform(_Iterator x, _Iterator end) : alloc(), m_coeafs(NULL), m_length(0) {
		if (end==x) {
			FastFourierTransform();
		} else if ((end - x) % 2 == 1) {
			throw math_exception("Input must be pairs of real and imaginary numbers.");
		} else {
			doTheFFT(x,end,1);
		}
	}

	virtual ~FastFourierTransform() {
		alloc.deallocate(m_coeafs,m_length);
	}

	virtual const complex_type f(const float_type& _x) {
		float_type x = _x;
		if (x > 1)
			x -= Math::floor(x);
		else if (x < 0)
			x = Math::ceil(x) - x;

		return complex_type();
	}

	virtual float_type getFundamentalFrequency() {
		return float_type(0);
	}

	virtual void normalize() {
	}

	virtual void lowPass(float_type cutoff, float_type w) {
	}

	virtual void highPass(float_type cutoff, float_type w) {
	}

	virtual void bandPass(float_type center, float_type w) {
	}

	virtual size_t size() {
		return m_length/2;
	}

	virtual complex_type coefficient(size_t i) {
		float_type* cs = m_coeafs + i*2;
		return complex_type(*cs, *(cs+1));
	}

};


template <typename _Float, typename _Alloc = DEFAULT_ALLOCATOR<std::complex<_Float> > >
class FastFourierTransform2 : public Transform<_Float> {
public:
	typedef std::complex<_Float> complex_type;
	typedef std::vector<complex_type,_Alloc> array_type;
	typedef _Float float_type;
private:
	array_type m_coeffs;

	template<typename _Iterator>
	array_type do_fft(_Iterator x, _Iterator end) {
		int N = end-x;

		// base case
		if (N == 1) return array_type( 1, *x);

		// radix 2 Cooley-Tukey FFT
		if (N % 2 != 0) { throw math_exception("N is not a power of 2"); }

		// fft of even terms
		array_type even = array_type(N/2);
		for (int k = 0; k < N/2; k++) {
			even[k] = x[2*k];
		}
		array_type q = do_fft(even.begin(),even.end());

		// fft of odd terms
		array_type odd = array_type(N/2);
		for (int k = 0; k < N/2; k++) {
			odd[k] = x[2*k + 1];
		}
		array_type r = do_fft(odd.begin(),odd.end());

		// combine
		array_type y = array_type(N);
		for (int k = 0; k < N/2; k++) {
			double kth = -2 * k * Math::pi(float_type(1)) / N;
			complex_type wk = std::polar(float_type(1), kth);
			y[k]       = q[k] + (wk*r[k]);
			y[k + N/2] = q[k] - (wk*r[k]);
		}
		return y;
	}

public:
	FastFourierTransform2() : m_coeffs(1) {
	}

	FastFourierTransform2(const float_type* coeafs, int length) {
		m_coeffs = array_type(length,coeafs);
	}

	template<typename _T>
	FastFourierTransform2(const FastFourierTransform2<_T>& other) {
		m_coeffs = array_type(other.m_coeffs.size());
		copy_it(m_coeffs.begin(),other.m_coeffs.begin(),other.m_coeffs.size());
	}

	template<typename _Iterator>
	FastFourierTransform2(_Iterator x, _Iterator end) : m_coeffs(1) {
		if (end==x) {
		} else if ((end - x) % 2 == 1) {
			throw math_exception("Input must be pairs of real and imaginary numbers.");
		} else {
			m_coeffs = do_fft(x,end);
		}
	}

	virtual ~FastFourierTransform2() {
	}

	virtual const complex_type f(const float_type& _x) {
		float_type x = _x;
		if (x > 1)
			x -= Math::floor(x);
		else if (x < 0)
			x = Math::ceil(x) - x;
		complex_type r = m_coeffs[0];
		size_t N = m_coeffs.size();
		float_type t = x*Math::pi(float_type(1));
		complex_type eix = std::polar(float_type(1), t);
		complex_type eiy = eix;
		for (size_t i = 1; i < N; i++) {
			r += m_coeffs[i]*eiy;
			eiy *= eix;
		}
		return r;
	}

	virtual float_type getFundamentalFrequency() {
		return float_type(0);
	}

	virtual void normalize() {
	}

	virtual void lowPass(float_type cutoff, float_type w) {
	}

	virtual void highPass(float_type cutoff, float_type w) {
	}

	virtual void bandPass(float_type center, float_type w) {
	}

	virtual size_t size() {
		return m_coeffs.size();
	}

	virtual complex_type coefficient(size_t i) {
		return m_coeffs[i];
	}

	template<typename _T>
	FastFourierTransform2<_Float>& operator = (const FastFourierTransform2<_T>& other) {
		m_coeffs = array_type(other.m_coeffs.size());
		copy_it(m_coeffs.begin(),other.m_coeffs.begin(),other.m_coeffs.size());
	}

};

}

}

}

#endif /* FFT_HPP_ */
