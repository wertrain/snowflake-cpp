#include <iostream>
#include <snowflake/snowflake.h>

int main()
{
    std::cout << snowflake::generate() << std::endl;

    return 0;
}