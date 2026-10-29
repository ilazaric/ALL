#include <meta>

int value = 2;
// int& ref1 = extract<int&>(^^value);
int& ref2 = extract<int&>(object_of(^^value));
