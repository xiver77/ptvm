#include "ptvm.hpp"

int main()
{
	using namespace Instructions;
	using namespace RegisterTypes;
	using namespace WriteTypes;
	std::vector<unsigned char> test
	{
		FN, 0, 0,
		MRL, G, 1, 0, 0, 0, 10,
		CAL, 0, 1,
		CAL, 0, 2,
		WR, SGN, G, 0, WRL,
		RTN,
		FN, 0, 1,
		MRL, L, 1, 0, 0, 0, 20,
		MLC, L, 2, L, 1,
		MRR, L, 3, L, 2,
		MRL, L, 5, 0, 0, 0, 1,
		MRL, L, 4, 0, 0, 0, 72,
		MMR, L, 3, L, 4, 1,
		ADD, L, 3, L, 5,
		MRL, L, 4, 0, 0, 0, 105,
		MMR, L, 3, L, 4, 1,
		ADD, L, 3, L, 5,
		MRL, L, 4, 0, 0, 0, 33,
		MMR, L, 3, L, 4, 1,
		ADD, L, 3, L, 5,
		MRL, L, 4, 0, 0, 0, 0,
		MMR, L, 3, L, 4, 1,
		WR, STR, L, 2, WRL,
		FRE, L, 2,
		RTN,
		FN, 0, 2,
		MRL, L, 1, 0, 0, 0, 1,
		MRL, G, 0, 0, 0, 0, 1,
		EQ, G, 1, L, 1, LNT,
		GO2, 0,
		RTN,
		LBL, 0,
		MIN, G, 0,
		ADD, G, 0, G, 1,
		MRR, L, 1, G, 1,
		MRR, G, 1, G, 0,
		CAL, 0, 2,
		SML, G, 0, L, 1,
		RTN,
	};
	Interpreter::vectorToFile("test.ptb", test.begin(), test.end());
	test = Interpreter::fileToVector("test.ptb");
	Interpreter().start(test.begin(), test.end());
	return EXIT_SUCCESS;
}
