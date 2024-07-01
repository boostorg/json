#include <cstdlib>
#include <boost/json/value.hpp>
#include <boost/json/string.hpp>


using namespace boost::json;


int main()
{
    string js;
goto STRING1;
STRING1:

    js = "1";
goto STRING2;
STRING2:

    js = "this is a very long string, unusually long even, definitely not short";
goto STRING3;
STRING3:

    return EXIT_SUCCESS;
}
