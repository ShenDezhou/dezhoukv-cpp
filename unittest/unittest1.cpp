#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN

#include <boost/test/auto_unit_test.hpp>
#include <boost/config/auto_link.hpp>


BOOST_AUTO_TEST_CASE(test1)
{
	int a = 3;
	int b = 4;
	int c = a * b;
	BOOST_CHECK(c == 12);
	BOOST_CHECK_EQUAL(c, 12);
}

BOOST_AUTO_TEST_CASE(test2)
{
	int a = 3;
	int b = 4;
	int c = a * b;
	BOOST_CHECK(c == 12);
	BOOST_CHECK_EQUAL(c, 12);
}
