/*
 * test_codec.cpp
 *
 *  Created on: 09/06/2015
 *      Author: ALAN
 */

#include <utils/time.hpp>
#include <utils/test/test_tools.hpp>
#include <utils/codec/encoder.hpp>
#include <utils/crypt/md5.hpp>
#include <utils/crypt/sha1.hpp>
#include <utils/crypt/sha2.hpp>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;
using namespace utils;
using namespace utils::codec;
using namespace utils::crypt;

class Test_Hex : public utils::test::TestCase {
public:

	Test_Hex() {
		ADD_TEST_FUNCTION(Test_Hex, Test_Hex::test_encode);
		ADD_TEST_FUNCTION(Test_Hex, Test_Hex::test_decode);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test_encode() {
		HexEncoder<char> enc(true);
		char dest[128];
		const char* value = "0123456789;";
		memset(dest,0,128);
		enc.encode((const unsigned char*)value,strlen(value),dest,128);
		ASSERT3(strcmp(dest,"303132333435363738393b")==0);
		// "\xA8" written as an escape keeps this file pure ASCII while holding
		// the byte exactly; the expected hex below asserts ...2a26 a8 2524...
		value = "+_)(*&\xA8%$#@!^LKJGH";
		memset(dest,0,128);
		enc.encode((const unsigned char*)value,strlen(value),dest,128);
		ASSERT3(strcmp(dest,"2b5f29282a26a825242340215e4c4b4a4748")==0);
	}

	void test_decode() {
		HexEncoder<char> enc;
		char dest[128];
		const char* value = "303132333435363738393b";
		memset(dest,0,128);
		enc.decode(value,strlen(value),(unsigned char*)dest,128);
		ASSERT3(strcmp(dest,"0123456789;")==0);
		value = "2b5f29282a26a825242340215e4c4b4a4748";
		memset(dest,0,128);
		enc.decode(value,strlen(value),(unsigned char*)dest,128);
		ASSERT3(strcmp(dest,"+_)(*&\xA8%$#@!^LKJGH")==0);
	}

};

class Test_Base32 : public utils::test::TestCase {
public:

	Test_Base32() {
		ADD_TEST_FUNCTION(Test_Base32, Test_Base32::test_encode);
		ADD_TEST_FUNCTION(Test_Base32, Test_Base32::test_decode);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test_encode(const char* src, const char* expect, Base32Encoder<char>& enc) {
		char dest[128];
		memset(dest,0,128);
		enc.encode((const unsigned char*)src,strlen(src),dest,128);
		char msg[256] = {0,};
		sprintf(msg,"Base32(%s) expected '%s', but '%s' found.",src,expect,dest);
		ASSERT(strcmp(dest,expect)==0,msg);
	}

	void test_decode(const char* expect, const char* src, Base32Encoder<char>& enc) {
		char dest[128];
		memset(dest,0,128);
		enc.decode(src,strlen(src),(unsigned char*)dest,128);
		char msg[256] = {0,};
		sprintf(msg,"Decode Base32(%s) expected '%s', but '%s' found.",src,expect,dest);
		ASSERT(strcmp(dest,expect)==0,msg);
	}

	void test_encode() {
		Base32Encoder<char> enc(false);
		Base32Encoder<char> ench(true);
		test_encode("","",enc);
		test_encode("f","MY======",enc);
		test_encode("fo","MZXQ====",enc);
		test_encode("foo","MZXW6===",enc);
		test_encode("foob","MZXW6YQ=",enc);
		test_encode("fooba","MZXW6YTB",enc);
		test_encode("foobar","MZXW6YTBOI======",enc);
		test_encode("","",ench);
		test_encode("f","CO======",ench);
		test_encode("fo","CPNG====",ench);
		test_encode("foo","CPNMU===",ench);
		test_encode("foob","CPNMUOG=",ench);
		test_encode("fooba","CPNMUOJ1",ench);
		test_encode("foobar","CPNMUOJ1E8======",ench);
	}

	void test_decode() {
		Base32Encoder<char> enc(false);
		Base32Encoder<char> ench(true);
		test_decode("","",enc);
		test_decode("f","MY======",enc);
		test_decode("fo","MZXQ====",enc);
		test_decode("foo","MZXW6===",enc);
		test_decode("foob","MZXW6YQ=",enc);
		test_decode("fooba","MZXW6YTB",enc);
		test_decode("foobar","MZXW6YTBOI======",enc);
		test_decode("","",ench);
		test_decode("f","CO======",ench);
		test_decode("fo","CPNG====",ench);
		test_decode("foo","CPNMU===",ench);
		test_decode("foob","CPNMUOG=",ench);
		test_decode("fooba","CPNMUOJ1",ench);
		test_decode("foobar","CPNMUOJ1E8======",ench);
	}

};

class Test_Base64 : public utils::test::TestCase {
public:

	Test_Base64() {
		ADD_TEST_FUNCTION(Test_Base64, Test_Base64::test_encode);
		ADD_TEST_FUNCTION(Test_Base64, Test_Base64::test_decode);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test_encode(const char* src, const char* expect, Base64Encoder<char>& enc) {
		char dest[128];
		memset(dest,0,128);
		enc.encode((const unsigned char*)src,strlen(src),dest,128);
		char msg[256] = {0,};
		sprintf(msg,"Base64(%s) expected '%s', but '%s' found.",src,expect,dest);
		ASSERT(strcmp(dest,expect)==0,msg);
	}

	void test_decode(const char* expect, const char* src, Base64Encoder<char>& enc) {
		char dest[128];
		memset(dest,0,128);
		enc.decode(src,strlen(src),(unsigned char*)dest,128);
		char msg[256] = {0,};
		sprintf(msg,"Decode Base64(%s) expected '%s', but '%s' found.",src,expect,dest);
		ASSERT(strcmp(dest,expect)==0,msg);
	}

	void test_encode() {
		Base64Encoder<char> enc(false);
		test_encode("","",enc);
		test_encode("f","Zg==",enc);
		test_encode("fo","Zm8=",enc);
		test_encode("foo","Zm9v",enc);
		test_encode("foob","Zm9vYg==",enc);
		test_encode("fooba","Zm9vYmE=",enc);
		test_encode("foobar","Zm9vYmFy",enc);
	}

	void test_decode() {
		Base64Encoder<char> enc(false);
		test_decode("","",enc);
		test_decode("f","Zg==",enc);
		test_decode("fo","Zm8=",enc);
		test_decode("foo","Zm9v",enc);
		test_decode("foob","Zm9vYg==",enc);
		test_decode("fooba","Zm9vYmE=",enc);
		test_decode("foobar","Zm9vYmFy",enc);
	}

};

class Test_MD5 : public utils::test::TestCase {
public:

	Test_MD5() {
		ADD_TEST_FUNCTION(Test_MD5, Test_MD5::test_crypt);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test(const char* src, const char* expct) {
		MD5Hasher md5;
		HexEncoder<char> hex(true);
		char dest[128];
		MD5Hasher::byte_type md5value[16];
		md5.update((MD5Hasher::byte_type*) src,strlen(src));
		md5.finalize(md5value);
		memset(dest,0,128);
		hex.encode(md5value,16,dest,128);
		char msg[256] = {0,};
		sprintf(msg,"MD5(%s) expected '%s', but '%s' found.",src,expct,dest);
		ASSERT(strcmp(dest,expct)==0,msg);
	}

	void test_crypt() {
		test("","d41d8cd98f00b204e9800998ecf8427e");
		test("a","0cc175b9c0f1b6a831c399e269772661");
		test("abc","900150983cd24fb0d6963f7d28e17f72");
		test("message digest","f96b697d7cb7938d525a2f31aaf161d0");
		test("abcdefghijklmnopqrstuvwxyz","c3fcd3d76192e4007dfb496cca67e13b");
		test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789","d174ab98d277d9f5a5611c2c9f419d9f");
		test("12345678901234567890123456789012345678901234567890123456789012345678901234567890","57edf4a22be3c955ac49da2e2107b67a");
	}

};

class Test_SHA1 : public utils::test::TestCase {
public:

	Test_SHA1() {
		ADD_TEST_FUNCTION(Test_SHA1, Test_SHA1::test_crypt);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test(const char* src, const char* expct) {
		SHA1Hasher sha1;
		HexEncoder<char> hex(true);
		char dest[128];
		SHA1Hasher::byte_type sha1value[20];
		sha1.update((SHA1Hasher::byte_type*) src,strlen(src));
		sha1.finalize(sha1value);
		memset(dest,0,128);
		hex.encode(sha1value,20,dest,128);
		char msg[256] = {0,};
		sprintf(msg,"SHA1(%s) expected '%s', but '%s' found.",src,expct,dest);
		ASSERT(strcmp(dest,expct)==0,msg);
	}

	void test_crypt() {
		test("", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
		test("a", "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8");
		test("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
		test("JF:Q<(q#qY", "7ad3a22985daa4f42a9463e05cce27d9557ab8bb");
		test("oXAQQi:2L8*qntKXj", "d2075e94c0942ae9d56cd814a45365b12ee7e3f1");
		test("$gUTJA8@j.FweL!0DT0t.031VH:hNIwVl", "8034dc5c5a8b75955ca572d29ba3c2fd0533a832");
		test("Uj%?8(u&j)>zlw8!aE0:x", "7c001a499d8854380999ab693a7aad50a2296b05");
		test("NcSTq27AYwEML>tm1MhdX$Ar_$7KXopI?a", "10c29883ccc0978aab13b836c18e0b4786d236b7");
		test("KL2p7>N,tRr0FFefc2cacW4lzanO3cLsj", "eae7d675d6246c575c5108ed15fb885ef74f216e");
		test("0UYR?DSBRl&W_O%6HU$5&?A9fj4", "8ee7f26e44f770415d6afa0d3e895d2bb18b5df8");
		test("($*", "5476238bd8d27c093dc455411f2ccf603f414a1d");
		test("bj!#_iTw9iwR.wOy3qb9t)!", "8a3940ee292ee9c68e9dd4210f1c9adb78381bf6");
		test("ef)TJK2+", "d96742e09e7ead912ccd511ea90ff5273ce9c1f2");
	}

};

class Test_SHA256 : public utils::test::TestCase {
public:

	Test_SHA256() {
		ADD_TEST_FUNCTION(Test_SHA256, Test_SHA256::test_crypt);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test(const char* src, const char* expct) {
		SHA256Hasher sha1;
		HexEncoder<char> hex(true);
		char dest[128];
		SHA256Hasher::byte_type sha1value[32];
		sha1.update((SHA256Hasher::byte_type*) src,strlen(src));
		sha1.finalize(sha1value);
		memset(dest,0,128);
		hex.encode(sha1value,32,dest,128);
		char msg[256] = {0,};
		sprintf(msg,"SHA256(%s) expected '%s', but '%s' found.",src,expct,dest);
		ASSERT(strcmp(dest,expct)==0,msg);
	}

	void test_crypt() {
		test("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
		test("a", "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb");
		test("abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
		test("zF*4lmE5hWn)&_++AS", "a10e7509b041453c748ea99bbc92a5a1ed6360613af1180624da9844c6c1e8b5");
		test(")7l$eCOZ%YP27jV*ryeie&lW", "dad230fa453a7fe251a0176437c14b062b684951cd2d389929959ae6e3ea4ef5");
		test("u!huev+1CFu)betoA11?lJL11OS9", "ba0e55a0d475027a278f4256cdb347eafdcbe434902fdaa5980774fefd7a8d4f");
		test("DkgUnSAVB6yBF%XWTMW,zsg:Fv", "90e90959812358d5fba0ade21e93fa15598d8f5b5a9b5a56ab91e13237353483");
		test("+ZrHp<KGKhpv3cKW+x.&KZrH6AF2Y", "02deba9fc290ecc3f8364342e3140559a3640fa944a534f6b72989962d6b6dfc");
		test("DL__", "374ffce2bc44337a34998f7ee01a6c59fe97cc713fae98ff95c679cf2becee70");
		test("$dOdR", "6035fb5fc041f35725c0bb959a07e8d8c1f1ec109903990c316141e8bcbf3b5d");
		test("bcrc8M8e!fXcFSAkT)T.1yz6BSv", "0af5a38d02357e9519f6399cfccf16e951a0935c37ba0a12c4163c069d8aa186");
		test("*kTyCAz5>,xzQ7DAIOfZmVA", "00c7f14349d6626a14cb4c7373e52605f818a44e7e8318d564f582927101a1eb");
		test("d5", "25f682044b5badaff8b296bf6fd676214968b238fcff5559e64f38c88bb9790f");
	}

};

class Test_SHA384 : public utils::test::TestCase {
public:

	Test_SHA384() {
		ADD_TEST_FUNCTION(Test_SHA384, Test_SHA384::test_crypt);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test(const char* src, const char* expct) {
		SHA384Hasher sha;
		HexEncoder<char> hex(true);
		char dest[256];
		SHA384Hasher::byte_type shavalue[48];
		sha.update((SHA384Hasher::byte_type*) src,strlen(src));
		sha.finalize(shavalue);
		memset(dest,0,256);
		hex.encode(shavalue,48,dest,256);
		char msg[512] = {0,};
		sprintf(msg,"SHA384(%s) expected '%s', but '%s' found.",src,expct,dest);
		ASSERT(strcmp(dest,expct)==0,msg);
	}

	void test_crypt() {
		test("", "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
		test("a", "54a59b9f22b0b80880d8427e548b7c23abd873486e1f035dce9cd697e85175033caa88e6d57bc35efae0b5afd3145f31");
		test("abc", "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7");
		test("gkotvuszwjokszdnh", "7ff2ef6d3ef6400a53b74af51facca1c1e014f6934642a8942eb826a3b18f689e8b597d6c02978886de79d213efcad47");
		test("ksbvyucjwfipqnxradt", "c316f723e5c7621742e1aa8fd05defda57f3d56f37ce6d5573c5ebc6a9e456f069024f1265ad36d431a39ba83ffd9ae3");
		test("bkxqdkfdxltlijdnhte", "3b6f729f1fb6e7042a1414e17a48f9a2b069bc3ba0b69c0e38dd92c61c1dd0e68f783136828be2014c8a55b6899b1db2");
		test("szjk", "222053396263374b76e1248cb203c9c466f791aad5f08123c78732ba685e258777b80410ffc336ade58caf6e74da97b7");
		test("n ozqornvqoshjmjrqowrqklsljeal", "cfe417a8f44285530f16e104f093c660e38b5553fd0271f7817b0fe643065f776381f5a7e24ce3b7df450b87a98a3e67");
		test("ugxhsmlemqxdaxclcytcbfxaadpuzfvc wkubw", "0e145e3046d6843c009e5c9e6b7bae7d372b283b6954013369235245d982ce8bd0b2d4e1fa4f9df9d08ae136089adf69");
		test("nruhtzpjpy", "58c31ca6255dad25d6606e1e65efd1bb55c159a457a8e0aa84c7866defd28d06371dd638e627c0de679a8ad04170c4b3");
		test("fldjsiuy", "16a179f70368e92e7117461ac414947460db0efd8528d194a235f0014d16ee2f6c55a9fe5b3e1e0ebbb494c65a3852d0");
		test("ebeh q  okcwhzjnz", "3e4598005c85a8a6494fa208575e9be6fe0a271b10d3a82b505d709b619d55f34baeecff9a475f7a0690cb623c9e41ea");
		test("iicr fvytkxhspl cthgaqsoizhytpdnc  cyi", "899bd27f0cc6679354adc32abd6a06aa40f6fbdf4818270cf1d4aa9c8d99cbced8e2b5417e093306ac2e7ee773f96746");
	}

};

class Test_SHA512 : public utils::test::TestCase {
public:

	Test_SHA512() {
		ADD_TEST_FUNCTION(Test_SHA512, Test_SHA512::test_crypt);
	}

	virtual void prepare_test() { }
	virtual void close_test() { }

	void test(const char* src, const char* expct) {
		SHA512Hasher sha;
		HexEncoder<char> hex(true);
		char dest[256];
		SHA512Hasher::byte_type shavalue[64];
		sha.update((SHA512Hasher::byte_type*) src,strlen(src));
		sha.finalize(shavalue);
		memset(dest,0,256);
		hex.encode(shavalue,64,dest,256);
		char msg[512] = {0,};
		sprintf(msg,"SHA512(%s) expected '%s', but '%s' found.",src,expct,dest);
		ASSERT(strcmp(dest,expct)==0,msg);
	}

	void test_crypt() {
		test("", "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
		test("a", "1f40fc92da241694750979ee6cf582f2d5d7d28e18335de05abc54d0560e0f5302860c652bf08d560252aa5e74210546f369fbbbce8c12cfc7957b2652fe9a75");
		test("abc", "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
		test("klyttwmuwejlbdfdutxnbc", "1f48571c6e4e476a72bd3e18037da0df55dd7510fe2beccf419bfb2fb1da7bfb8c538e97a2ee05d78fddf038a5ac34b1101e9b7f7d7dc02b507c515aecd2798f");
		test("voamjifyunsx", "ee1d8c742971acfd82ea85cd03ac80fd444cf36fc93f7aa5df270212376e78ab0d9634a39572577a8439384a6dfee31266987c1970f46b774ced23e4cda5fe15");
		test("wyfgvsqmymhpg", "9cc300edce246c17846af3d866ff0aa246df922156049341912bcc1d4936132d9230a3914f18425e56ac8e3ecebdbc311043cbf27bb725f3c083047666ee5019");
		test("wkefzgjllqluybyz cgpuwojop mtqhr", "c10130b7db1345d2e6b4e58d78b1f265894e3904e83fc7e7a2d476bff8f5aa4603ff3eacbf84686f5fd5fdb090d2a5f155e448d8f0c7c023ef386c27f94df3e6");
		test("pmka k", "e91b247714bc48a1012c1951a39c34210669f9666756b576db0e5c8bb3c1b254f8179e9aec027ddb1a9bd5bc1824af45c3cf56bdabfd8d04c4f6ad77535853d4");
		test("", "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
		test("nrhsntkqkkwpxqtx", "969874bac746c28cc1e0ff3a448be64cf42e4fc81514e5b46cb118b7d7ee58d8a60190ef923b323b74000c5b7dfea81b2c834065b27178f7d4c6ce7626285dab");
		test("kpivnecccloaznmzqrpkbvkrojv", "bc063f28340e9a721a8d87df985e8e4e1cfbfaa6584f9d11b82c7841692c9a4b5509e68438a2fb30892282160d55f2872816f4d73d7de5e8f8cb6184185dbd21");
		test("vjjwzaouhmqe", "672a5156f8386c54c5e5b6b32dc81679102841305f482f79f7877ce28bbbe7755ac653f40e755f9a24914066b3cc886219593091e0a6fe4b63f4007818b08dfb");
		test("driatjprege", "6e7b1328630220b8292f765171d82455357ae3cd042dd5bebf1d231841990853b0efeb5740472e78347bbca485c250c4009f186967e532d9f4838309761849c2");
	}

};

void register_test_codec() {
	ADD_TEST_CASE(Test_Hex);
	ADD_TEST_CASE(Test_Base32);
	ADD_TEST_CASE(Test_Base64);
	ADD_TEST_CASE(Test_MD5);
	ADD_TEST_CASE(Test_SHA1);
	ADD_TEST_CASE(Test_SHA256);
	ADD_TEST_CASE(Test_SHA384);
	ADD_TEST_CASE(Test_SHA512);
}
