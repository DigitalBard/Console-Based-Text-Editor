#include <iostream>
// ½Ç½À 4 - (2)

int& func(int& a) { return a; }

int main()
{
	int x = 1;

	std::cout << func(x)++ << std::endl;
	std::cout << x << std::endl;
}