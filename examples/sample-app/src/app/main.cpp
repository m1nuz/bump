#include <iostream>

#include <libsum/sum.h>
#include <libmul/mul.h>

int main( int argc, char *argv[] ) {
    std::cout << "Sample App" << '\n';
    std::cout << "Sum " << sum( 2, 3 ) << '\n';
    std::cout << "Mul " << mul( 2, 3 ) << '\n';

    return 0;
}
