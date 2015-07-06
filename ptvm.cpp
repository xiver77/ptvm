#include "ptvm.hpp"

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		Error::abort("must have 1 argument");
	}
	std::vector<unsigned char> test = Interpreter::fileToVector(argv[1]);
	Interpreter().start(test.begin(), test.end());
	return EXIT_SUCCESS;
}
