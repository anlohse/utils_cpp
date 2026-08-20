/*
 * test_commons.cpp
 *
 *  Created on: 25/03/2010
 *      Author: alan.lohse
 */

#include <utils/time.hpp>
#include <utils/test/test_tools.hpp>
#include <utils/math/giantint.hpp>
#include <utils/math/decimal.hpp>
#include <utils/math/transforms/fft.hpp>
#include <utils/math/statictics.hpp>
#include <cstdint>
#include <random>
#include <utils/conversion.hpp>
#include <iostream>
#include <string>

#include <math.h>

using namespace std;
using namespace utils;
using namespace utils::math;
using namespace utils::math::transforms;

class Test_Random : public utils::test::TestCase {
public:

	Test_Random() {
		ADD_TEST_FUNCTION(Test_Random, Test_Random::test_random);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test_random() {
		std::linear_congruential_engine<std::uint_fast32_t,76543,34567,0x7fffffff> rnd;
		t_double sum = 0;
		t_double mid = 0;
		t_double desv = 0;
		for (t_int i = 0; i < 16777216; i++) {
			t_int vl = rnd()%33554432 - 16777216;
			sum += (t_double)vl;
			desv += (((t_double)vl)/16777216)*(((t_double)vl)/16777216);
		}
		mid = sum/16777216;
		desv = ::sqrt(desv);
		cout << "The middle is " << (t_int)mid << endl;
		cout << "A good middle value would be 0" << endl;
		cout << "Deviation " << (t_int)desv << endl;
	}

};


class Test_giantint : public utils::test::TestCase {
public:
	giantint a;
	giantint b;
	giantint c;
	giantint d;
	giantint e;
	giantint f;
	giantint g;

	Test_giantint() {
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_constructors);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_comparations);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_addition);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_subtraction);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_multiplication);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_division);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_left_shift);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_right_shift);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_bitwise_operations);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_sqrt);
		ADD_TEST_FUNCTION(Test_giantint, Test_giantint::test_pow);
	}

	virtual void prepare_test() {
		a = giantint(0x9FFFFFFFLL);
		b = giantint(0xAEDCBA98LL);
		c = giantint(0x14EDCBA97LL);
		d = giantint("12345678901234567890");
		e = giantint("98765432109876543210");
		f = giantint("111111111011111111100");
		g = giantint("111111111016729166931");
	}
	virtual void close_test() {

	}

	void test_constructors() {
		giantint a(0x9FFFFFFFLL);
		giantint b(0xAEDCBA98LL);
		giantint d("12345678901234567890");
		giantint e("98765432109876543210");
		giantint f("111111111011111111100");
		giantint g("111111111011111111100",2);
		giantint h("123558Afeb236",16);
		ASSERT(a.to_string() == "2684354559", string("expected 2684354559 but found ")+a.to_string() );
		ASSERT(b.to_string() == "2933701272", string("expected 2933701272 but found ")+b.to_string() );
		ASSERT(d.to_string() == "12345678901234567890", string("expected 12345678901234567890 but found ")+d.to_string() );
		ASSERT(e.to_string() == "98765432109876543210", string("expected 98765432109876543210 but found ")+e.to_string() );
		ASSERT(f.to_string() == "111111111011111111100", string("expected 111111111011111111100 but found ")+f.to_string() );
		t_int sign;
		ASSERT(g.to_string(2,sign) == "111111111011111111100", string("expected 111111111011111111100 but found ")+g.to_string(2,sign) );
		ASSERT(h.to_string(16,sign) == "123558AFEB236", string("expected 123558AFEB236 but found ")+h.to_string(16,sign) );
	}

	void test_comparations() {
		ASSERT3(a != b);
		ASSERT3(a < b);
		ASSERT3(a <= b);
		ASSERT3(!(a == b));
		ASSERT3(!(a > b));
		ASSERT3(!(a >= b));
		ASSERT3(d != e);
		ASSERT3(d < e);
		ASSERT3(d <= e);
		ASSERT3(!(d == e));
		ASSERT3(!(d > e));
		ASSERT3(!(d >= e));
		ASSERT3(a >= giantint::zero());
		ASSERT3(b >= giantint::zero());
		ASSERT3(d >= giantint::zero());
		ASSERT3(e >= giantint::zero());
		giantint me("-98765432109876543210");
		ASSERT3(-e == me);
	}

	void test_addition() {
		giantint me("-98765432109876543210");
		ASSERT2(a + b, c);
		ASSERT2(d + e, f);
		ASSERT2(b + a, c);
		ASSERT2(e + d, f);
		ASSERT2(f + me, d);
		ASSERT2(f + -e, d);
		ASSERT2(c + -a, b);
		ASSERT2(c + -b, a);
		ASSERT2(a + b + d + e, g);
		ASSERT2(g + me, a + b + d);
		ASSERT2(g + -e, a + b + d);
		ASSERT2(g + -e, a + b + d);
		ASSERT2(a + b + d, d + b + a);
		ASSERT2(g + -e, a + b + d + e + me);
	}

	void test_subtraction() {
		giantint me("-98765432109876543210");
		ASSERT2(c - b, a);
		ASSERT2(f - e, d);
		ASSERT2(c - a, b);
		ASSERT2(f - d, e);
		ASSERT2(a - c, -b);
		ASSERT2(d - f, -e);
		ASSERT2(a - a, giantint::zero());
		ASSERT2(d - me, f);
		ASSERT2(d - -e, f);
		ASSERT2(b - -a, c);
		ASSERT2(a - -b, c);
		ASSERT2(g - b - d - e, a);
	}

	void test_multiplication() {
		giantint x("11111111111111111111");
		giantint y("-11111111111111111111");
		giantint dxe("1219326311370217952237463801111263526900");
		giantint xxy("-123456790123456790120987654320987654321");
		ASSERT2(d * e, dxe);
		ASSERT2(x * y, xxy);
	}

	void test_division() {
		giantint x("123456790123456790120987654320987654321");
		giantint y("11111111111111111111");
		giantint dxe("1219326311370217952237463801111263526900");
		ASSERT2(x / y, y);
		ASSERT2(x % y, giantint::zero());
		ASSERT2(dxe / e, d);
		ASSERT2(dxe / d, e);
		giantint w("885522996633774411");
		giantint z("1115334468842");
		giantint wxz("987654321098765432109876543210");
		giantint wrz("858495419540141148");
		ASSERT2(wxz / w, z);
		ASSERT2(wxz % w, wrz);
		giantint p("987654321987654321987654321987654321987654321987654321987654321");
		giantint q("219876543219876543219876543");
		giantint pdq("4491858510800763317440369257523092497");
		giantint prq("90402129090402130078056450");
		ASSERT2(p / q, pdq);
		ASSERT2(p % q, prq);
		giantint r("21987654321987654321987654321987654321");
		giantint s(123456);
		giantint rds("178101139855395074536577034101118");
		giantint rrs("30513");
		ASSERT2(r / s, rds);
		ASSERT2(r % s, rrs);
	}

	void test_left_shift() {
		giantint als1("5368709118");
		giantint dls5("395061724839506172480");
		giantint als35("92233720334188019712");
		giantint dls35("424194297021757464897933803520");
		giantint x("11");
		giantint xls33ls3("755914244096");
		ASSERT2((a << 1),als1);
		ASSERT2((d << 5),dls5);
		ASSERT2((a << 35),als35);
		ASSERT2((d << 35),dls35);
		ASSERT2((x << 33) << 3,xls33ls3);
	}

	void test_right_shift() {
		giantint ars1("1342177279");
		giantint drs5("385802465663580246");
		giantint drs35("359306545");
		giantint x("11");
		giantint xls33ls3("755914244096");
		ASSERT2((a >> 1),ars1);
		ASSERT2((d >> 5),drs5);
		ASSERT2((d >> 35),drs35);
		ASSERT2((xls33ls3 >> 3) >> 33,x);
	}

	void test_bitwise_operations() {
		giantint x("98416549841964841");
		giantint y("6816571");
		giantint xory("98416549844062011");
		giantint xandy("4719401");
		giantint xxory("98416549839342610");
		giantint notx("18348327523867586774");
		ASSERT2((x | y), xory);
		ASSERT2((x & y), xandy);
		ASSERT2((x ^ y), xxory);
		ASSERT2((~x), notx);
	}

	void test_sqrt() {
		giantint x("123456790123456790120987654320987654321");
		giantint y("11111111111111111111");
		ASSERT2(x.sqrt(), y);
	}

	void test_pow() {
		giantint x("123456790123456790120987654320987654321");
		giantint y("11111111111111111111");
		ASSERT2(y.pow(2), x);
	}

};

class Test_decimal : public utils::test::TestCase {
public:

	decimal a;
	decimal b;
	decimal c;
	decimal d;
	decimal e;
	decimal f;

	Test_decimal() {
		ADD_TEST_FUNCTION(Test_decimal, Test_decimal::test_constructors);
		ADD_TEST_FUNCTION(Test_decimal, Test_decimal::test_division);
		ADD_TEST_FUNCTION(Test_decimal, Test_decimal::test_sqrt);
	}

	virtual void prepare_test() {
		a = decimal("12345.678");
		b = decimal("123.4578");
		c = decimal("99.999174");
		d = decimal("1");
		e = decimal("99");
		f = decimal("0.01");
	}
	virtual void close_test() { }

	void test_constructors() {
		decimal d1(1.0);
		decimal a1(12345678,3);
		decimal f1(0.01);
		decimal c1(99999174LL,6);
		ASSERT2(d1, d);
		ASSERT2(a1, a);
		ASSERT2(f1, f);
		ASSERT2(c1, c);
	}

	void test_division() {
		ASSERT2(a / b, c);
		ASSERT2(d / e, f);
	}

	void test_sqrt() {
		decimal x("2");
		decimal sr2("1.41421356237309504880168872420969807856967187537694807317667973799");
		x.scale(65);
		ASSERT2(x.sqrt(), sr2);
	}

};

class Test_FFT : public utils::test::TestCase {
public:

	Test_FFT() {
		ADD_TEST_FUNCTION(Test_FFT, Test_FFT::test_fft);
		ADD_TEST_FUNCTION_EX(Test_FFT, Test_FFT::test_fft_int,"Type must be floating point.");
		ADD_TEST_FUNCTION(Test_FFT, Test_FFT::test_dft_performance);
		ADD_TEST_FUNCTION(Test_FFT, Test_FFT::test_fft_performance);
		ADD_TEST_FUNCTION(Test_FFT, Test_FFT::test_fft2_performance);
	}

	std::complex<double> data[256];

	virtual void prepare_test() {
		std::complex<double>* it = data;
		std::complex<double>* end = data+256;
		for (; it < end; it++) {
			*it = std::complex<double>(Math::sin(2*Math::pi(1.0)*(it - data)/256.0),0);
		}
	}
	virtual void close_test() { }

	void test_dft_performance() {
		std::complex<double>* it = data;
		std::complex<double>* end = data+256;
		DiscreteFourierTransform<double> fft(it,end);
	}

	void test_fft2_performance() {
		std::complex<double>* it = data;
		std::complex<double>* end = data+256;
		FastFourierTransform2<double> fft(it,end);
	}

	void test_fft_performance() {
		double* it = (double*)data;
		double* end = ((double*)data)+512;
		FastFourierTransform<double> fft(it,end);
	}

	void test_fft_int() {
		DiscreteFourierTransform<int> ffti;
	}

	void test_fft() {
		long double a = -1;
		a = Math::abs(a);
		cout << "a: " << a << endl;
		std::complex<double>* it = data;
		std::complex<double>* end = data+256;
		DiscreteFourierTransform<double> fft(it,end);
//		double* it = (double*)data;
//		double* end = ((double*)data)+512;
//		FastFourierTransform<double> fft(it,end);
		statistics<double> error;
		for (int i = 0; i < 256; i++) {
//			cout << i << "\t" << data[i].real() << "\t" << fft.f(i/256.0).real() << endl;
			error.push(data[i].real()-fft.f(i/256.0).real());
			cout << i << "\t" << fft.coefficient(i).real() << "\t" << fft.coefficient(i).imag() << endl;
		}
		cout << "AVG error: " << error.mean() << endl;
		cout << "Std Deviation error: " << error.standard_deviation() << endl;
		// data is 256 complex samples and the transform is built over all of
		// them, so size() is 256. The old expectation of 2048 was left over
		// from an earlier version of this test that ran a FastFourierTransform
		// over 512 doubles.
		ASSERT2(fft.size(), 256u);
	}

};

class Test_Conversion : public utils::test::TestCase {
public:

	Test_Conversion() {
		ADD_TEST_FUNCTION(Test_Conversion, Test_Conversion::test_conversion);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	template<typename _Int>
	void test_to_string(_Int input,const std::string& expected) {
		ostringstream s;
		std::string out = utils::to_string<_Int,std::string>(input);
		s << "Error converting '" << input << "' to string. Expected: '" << expected << "'" << " but '" << out << "' found.";
		ASSERT(out == expected, s.str().c_str());
	}

	template<typename _Int>
	void test_from_string(const std::string& input, _Int expected) {
		ostringstream s;
		_Int out = utils::from_string<_Int>(input);
		s << "Error parsing from '" << input << "' to string. Expected: '" << expected << "'" << " but '" << out << "' found.";
		ASSERT(out == expected, s.str().c_str());
	}

	void test_conversion() {
		test_to_string(1,"1");
		test_to_string(1234567890123LL,"1234567890123");
		test_to_string(1.234f,"1.234");
		test_to_string(1.23456789,"1.23456789");
		test_from_string<int>("1",1);
		test_from_string<__int64>("1234567890123",1234567890123LL);
		test_from_string<float>("1.234",1.234f);
		test_from_string<double>("1.23456789",1.23456789);
	}

};



void register_test_commons() {
	ADD_TEST_CASE(Test_Random);
	ADD_TEST_CASE(Test_giantint);
	ADD_TEST_CASE(Test_decimal);
	ADD_TEST_CASE(Test_FFT);
	ADD_TEST_CASE(Test_Conversion);
}

