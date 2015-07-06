#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <stack>

class Stdout
{
public:
	template<typename T, typename... Ts>
	static void print(const T& t, const Ts&... ts)
	{
		std::cout << t << std::flush;
		print(ts...);
	}

	static void print()
	{
	}

	template<typename... Ts>
	static void printLine(const Ts&... ts)
	{
		print(ts..., '\n');
	}
};

class Error
{
public:
	template<typename... Ts>
	static void abort(const Ts&... ts)
	{
		Stdout::printLine(ts...);
		throw;
	}
};

class Math
{
public:
	static unsigned long divideRoundUp(unsigned long a, unsigned long b)
	{
		return a / b + (a % b > 0);
	}
};

class VirtualHeap
{
private:
	enum
	{
		OCCUPIED,
		FREE,
	};

	std::vector<unsigned char> mVector;
	std::vector<unsigned long> mNotebook;
	std::vector<unsigned long>::iterator mIterator;

public:
	VirtualHeap()
	{
	}

	VirtualHeap(unsigned long u)
	: mVector(u)
	, mNotebook(Math::divideRoundUp(u, sizeof(unsigned long)))
	, mIterator(mNotebook.begin())
	{
		mNotebook.front() = mNotebook.back() = makeNote(mNotebook.size(), FREE);
		if (allocate(1) != 0)
		{
			Error::abort("There must be an error in VirtualHeap!");
		}
	}

private:
	unsigned long makeNote(unsigned long u, int n)
	{
		return u << 1 | n;
	}

	unsigned long extractSize(unsigned long u)
	{
		return u >> 1;
	}

	bool isFree(unsigned long u)
	{
		return u & FREE;
	}

public:
	unsigned char& operator[](unsigned long u)
	{
		return mVector[u];
	}

	unsigned long allocate(unsigned long u)
	{
		u = Math::divideRoundUp(u, sizeof(unsigned long));
		bool isFirst = true;
		while (true)
		{
			if (mIterator == mNotebook.end())
			{
				if (isFirst)
				{
					mIterator = mNotebook.begin();
					isFirst = false;
				}
				else
				{
					return 0;
				}
			}
			unsigned long size = extractSize(*mIterator);
			if (isFree(*mIterator))
			{
				if (size >= u)
				{
					unsigned long u2 = (mIterator - mNotebook.begin()) * sizeof(unsigned long);
					mIterator[u] = mIterator[size - 1] = makeNote(size - u, FREE);
					*mIterator = mIterator[u - 1] = makeNote(u, OCCUPIED);
					mIterator += u;
					return u2;
				}
			}
			mIterator += size;
		}
	}

	void free(unsigned long u)
	{
		u /= sizeof(unsigned long);
		unsigned long size = extractSize(mNotebook[u]);
		if (u != 0 && isFree(mNotebook[u - 1]))
		{
			if (u + size < mNotebook.size() && isFree(mNotebook[u + size]))
			{
				size += extractSize(mNotebook[u - 1]) + extractSize(mNotebook[size]);
			}
			else
			{
				size += extractSize(mNotebook[u - 1]);
			}
			u -= extractSize(mNotebook[u - 1]);
		}
		else
		{
			if (u + size < mNotebook.size() && isFree(mNotebook[u + size]))
			{
				size += extractSize(mNotebook[size]);
			}
		}
		mNotebook[u] = mNotebook[u + size - 1] = makeNote(size, FREE);
	}
};

namespace Instructions
{
	enum
	{
		FN,
		CAL,
		RTN,
		GO2,
		LBL,
		MRL,
		MRR,
		MRM,
		MMR,
		MLC,
		FRE,
		WR,
		WRL,
		ADD,
		SUB,
		SML,
		UML,
		SDV,
		UDV,
		SMD,
		UMD,
		LSH,
		RSH,
		AND,
		OR,
		XOR,
		EQ,
		SLT,
		ULT,
		SGT,
		UGT,
		LNT,
		BNT,
		MIN,
	};
}

namespace WriteTypes
{
	enum
	{
		SGN,
		UNS,
		CHR,
		STR,
	};
}

namespace RegisterTypes
{
	enum
	{
		L,
		G,
	};
}

class FunctionInfo
{
public:
	std::vector<unsigned char> byteCodes;
	std::vector<std::vector<unsigned char>::iterator> labels;

	FunctionInfo()
	{
	}

	FunctionInfo(int n)
	: byteCodes(n)
	, labels(0x100)
	{
	}
};

class Interpreter
{
private:
	std::vector<unsigned long> mGlobalRegisters;
	std::vector<FunctionInfo> mFunctionTable;
	VirtualHeap mHeap;
	std::vector<unsigned char>::iterator mByteCodeIterator;
	std::vector<std::vector<unsigned char>::iterator>::iterator mLabelIterator;
	std::array<std::vector<unsigned long>::iterator, 2> mRegisterIterators;
	std::stack<std::vector<unsigned char>::iterator> mByteCodeStack;
	std::stack<std::vector<std::vector<unsigned char>::iterator>::iterator> mLabelStack;
	std::stack<std::vector<unsigned long>> mRegisterStack;
	std::array<int, 100> mArgumentCounts;

public:
	Interpreter()
	: mGlobalRegisters(0x100)
	, mFunctionTable(0x10000)
	, mHeap(0x1000000)
	{
		using namespace Instructions;
		mRegisterIterators[RegisterTypes::G] = mGlobalRegisters.begin();
		mArgumentCounts[FN] = 3;
		mArgumentCounts[CAL] = 3;
		mArgumentCounts[RTN] = 1;
		mArgumentCounts[GO2] = 2;
		mArgumentCounts[LBL] = 2;
		mArgumentCounts[MRL] = 7;
		mArgumentCounts[MRR] = 5;
		mArgumentCounts[MRM] = 6;
		mArgumentCounts[MMR] = 6;
		mArgumentCounts[MLC] = 5;
		mArgumentCounts[FRE] = 3;
		mArgumentCounts[WR] = 4;
		mArgumentCounts[WRL] = 1;
		mArgumentCounts[ADD] = 5;
		mArgumentCounts[SUB] = 5;
		mArgumentCounts[SML] = 5;
		mArgumentCounts[UML] = 5;
		mArgumentCounts[SDV] = 5;
		mArgumentCounts[UDV] = 5;
		mArgumentCounts[SMD] = 5;
		mArgumentCounts[UMD] = 5;
		mArgumentCounts[LSH] = 5;
		mArgumentCounts[RSH] = 5;
		mArgumentCounts[AND] = 5;
		mArgumentCounts[OR] = 5;
		mArgumentCounts[XOR] = 5;
		mArgumentCounts[EQ] = 5;
		mArgumentCounts[SLT] = 5;
		mArgumentCounts[ULT] = 5;
		mArgumentCounts[SGT] = 5;
		mArgumentCounts[UGT] = 5;
		mArgumentCounts[LNT] = 1;
		mArgumentCounts[BNT] = 3;
		mArgumentCounts[MIN] = 3;
	}

private:
	static void abortDueToInvalidInstruction(int n)
	{
		Error::abort("invalid instruction ", n);
	}

	void fillLabels(unsigned int u)
	{
		using namespace Instructions;
		for (auto it = mFunctionTable[u].byteCodes.begin(); it != mFunctionTable[u].byteCodes.end();)
		{
			switch (*it)
			{
			case LBL:
				mFunctionTable[u].labels[it[1]] = it + 2;
			default:
				it += mArgumentCounts[*it];
			}
		}
	}

	void createFunctionTable(std::vector<unsigned char>::iterator it, std::vector<unsigned char>::iterator endIt)
	{
		using namespace Instructions;
		bool isEnd = false;
		while (!isEnd)
		{
			if (*it == FN)
			{
				it += mArgumentCounts[*it];
				auto it2 = it;
				while (true)
				{
					if (it2 == endIt)
					{
						isEnd = true;
						break;
					}
					if (*it2 == FN)
					{
						break;
					}
					else
					{
						it2 += mArgumentCounts[*it2];
					}
				}
				unsigned int u = it[-2] << 8 | it[-1];
				mFunctionTable[u] = FunctionInfo(it2 - it);
				std::copy(it, it2, mFunctionTable[u].byteCodes.begin());
				fillLabels(u);
				it = it2;
			}
			else
			{
				abortDueToInvalidInstruction(*it);
			}
		}
	}

	void doCAL(unsigned int u)
	{
		if (u != 0)
		{
			mByteCodeStack.push(mByteCodeIterator + mArgumentCounts[*mByteCodeIterator]);
			mLabelStack.push(mLabelIterator);
		}
		mRegisterStack.push(std::vector<unsigned long>(0x100));
		mByteCodeIterator = mFunctionTable[u].byteCodes.begin();
		mLabelIterator = mFunctionTable[u].labels.begin();
		mRegisterIterators[RegisterTypes::L] = mRegisterStack.top().begin();
	}

	void doRTN()
	{
		if (mByteCodeStack.empty())
		{
			std::exit(EXIT_SUCCESS);
		}
		mRegisterStack.pop();
		mByteCodeIterator = mByteCodeStack.top();
		mLabelIterator = mLabelStack.top();
		mRegisterIterators[RegisterTypes::L] = mRegisterStack.top().begin();
		mByteCodeStack.pop();
		mLabelStack.pop();
	}

	void doGO2(int n)
	{
		if (mRegisterIterators[RegisterTypes::L][0])
		{
			mByteCodeIterator = mLabelIterator[n];
		}
		else
		{
			mByteCodeIterator += mArgumentCounts[*mByteCodeIterator];
		}
	}

	void doMRL(int rt, int rn, unsigned long u)
	{
		mRegisterIterators[rt][rn] = u;
	}

	void doMRR(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] = mRegisterIterators[rt2][rn2];
	}

	void doMRM(int rt, int rn, int rt2, int rn2, int sz)
	{
		mRegisterIterators[rt][rn] = 0;
		for (int i = 0; i < sz; ++i)
		{
			mRegisterIterators[rt][rn] |= mHeap[mRegisterIterators[rt2][rn2] + i] << (sz - i - 1) * 8;
		}
	}

	void doMMR(int rt, int rn, int rt2, int rn2, int sz)
	{
		for (int i = 0; i < sz; ++i)
		{
			int n = (sz - i - 1) * 8;
			mHeap[mRegisterIterators[rt][rn] + i] = (mRegisterIterators[rt2][rn2] & 0xff << n) >> n;
		}
	}

	void doMLC(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] = mHeap.allocate(mRegisterIterators[rt2][rn2]);
	}

	void doFRE(int rt, int rn)
	{
		mHeap.free(mRegisterIterators[rt][rn]);
	}

	void doWR(int wt, int rt, int rn)
	{
		using namespace WriteTypes;
		long n;
		switch (wt)
		{
		case SGN:
			std::memcpy(&n, &mRegisterIterators[rt][rn], sizeof n);
			Stdout::print(n);
			return;
		case UNS:
			Stdout::print(mRegisterIterators[rt][rn]);
			return;
		case CHR:
			Stdout::print(static_cast<char>(mRegisterIterators[rt][rn]));
			return;
		case STR:
			Stdout::print(reinterpret_cast<char*>(&mHeap[mRegisterIterators[rt][rn]]));
			return;
		}
	}

	void doWRL()
	{
		Stdout::printLine();
	}

	void doADD(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] += mRegisterIterators[rt2][rn2];
	}

	void doSUB(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] -= mRegisterIterators[rt2][rn2];
	}

	void doSML(int rt, int rn, int rt2, int rn2)
	{
		long n, n2;
		std::memcpy(&n, &mRegisterIterators[rt][rn], sizeof n);
		std::memcpy(&n2, &mRegisterIterators[rt2][rn2], sizeof n2);
		n *= n2;
		std::memcpy(&mRegisterIterators[rt][rn], &n, sizeof n);
	}

	void doUML(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] *= mRegisterIterators[rt2][rn2];
	}

	void doSDV(int rt, int rn, int rt2, int rn2)
	{
		long n, n2;
		std::memcpy(&n, &mRegisterIterators[rt][rn], sizeof n);
		std::memcpy(&n2, &mRegisterIterators[rt2][rn2], sizeof n2);
		n /= n2;
		std::memcpy(&mRegisterIterators[rt][rn], &n, sizeof n);
	}

	void doUDV(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] /= mRegisterIterators[rt2][rn2];
	}

	void doSMD(int rt, int rn, int rt2, int rn2)
	{
		long n, n2;
		std::memcpy(&n, &mRegisterIterators[rt][rn], sizeof n);
		std::memcpy(&n2, &mRegisterIterators[rt2][rn2], sizeof n2);
		n %= n2;
		std::memcpy(&mRegisterIterators[rt][rn], &n, sizeof n);
	}

	void doUMD(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] %= mRegisterIterators[rt2][rn2];
	}

	void doLSH(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] <<= mRegisterIterators[rt2][rn2];
	}

	void doRSH(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] >>= mRegisterIterators[rt2][rn2];
	}

	void doAND(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] &= mRegisterIterators[rt2][rn2];
	}

	void doOR(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] |= mRegisterIterators[rt2][rn2];
	}

	void doXOR(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[rt][rn] ^= mRegisterIterators[rt2][rn2];
	}

	void doEQ(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[RegisterTypes::L][0] = mRegisterIterators[rt][rn] == mRegisterIterators[rt2][rn2];
	}

	void doSLT(int rt, int rn, int rt2, int rn2)
	{
		long n, n2;
		std::memcpy(&n, &mRegisterIterators[rt][rn], sizeof n);
		std::memcpy(&n2, &mRegisterIterators[rt2][rn2], sizeof n2);
		n = n < n2;
		std::memcpy(&mRegisterIterators[RegisterTypes::L][0], &n, sizeof n);
	}

	void doULT(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[RegisterTypes::L][0] = mRegisterIterators[rt][rn] < mRegisterIterators[rt2][rn2];
	}

	void doSGT(int rt, int rn, int rt2, int rn2)
	{
		long n, n2;
		std::memcpy(&n, &mRegisterIterators[rt][rn], sizeof n);
		std::memcpy(&n2, &mRegisterIterators[rt2][rn2], sizeof n2);
		n = n > n2;
		std::memcpy(&mRegisterIterators[RegisterTypes::L][0], &n, sizeof n);
	}

	void doUGT(int rt, int rn, int rt2, int rn2)
	{
		mRegisterIterators[RegisterTypes::L][0] = mRegisterIterators[rt][rn] > mRegisterIterators[rt2][rn2];
	}

	void doLNT()
	{
		mRegisterIterators[RegisterTypes::L][0] = !mRegisterIterators[RegisterTypes::L][0];
	}

	void doBNT(int rt, int rn)
	{
		mRegisterIterators[rt][rn] = ~mRegisterIterators[rt][rn];
	}

	void doMIN(int rt, int rn)
	{
		mRegisterIterators[rt][rn] = -mRegisterIterators[rt][rn];
	}

public:
	void start(std::vector<unsigned char>::iterator it, std::vector<unsigned char>::iterator endIt)
	{
		using namespace Instructions;
		createFunctionTable(it, endIt);
		doCAL(0);
		while (true)
		{
			auto& b = mByteCodeIterator;
			switch (*b)
			{
			case CAL:
				doCAL(b[1] << 8 | b[2]);
				continue;
			case RTN:
				doRTN();
				continue;
			case GO2:
				doGO2(b[1]);
				continue;
			case LBL:
				break;
			case MRL:
				doMRL(b[1], b[2], b[3] << 24 | b[4] << 16 | b[5] << 8 | b[6]);
				break;
			case MRR:
				doMRR(b[1], b[2], b[3], b[4]);
				break;
			case MRM:
				doMRM(b[1], b[2], b[3], b[4], b[5]);
				break;
			case MMR:
				doMMR(b[1], b[2], b[3], b[4], b[5]);
				break;
			case MLC:
				doMLC(b[1], b[2], b[3], b[4]);
				break;
			case FRE:
				doFRE(b[1], b[2]);
				break;
			case WR:
				doWR(b[1], b[2], b[3]);
				break;
			case WRL:
				doWRL();
				break;
			case ADD:
				doADD(b[1], b[2], b[3], b[4]);
				break;
			case SUB:
				doSUB(b[1], b[2], b[3], b[4]);
				break;
			case SML:
				doSML(b[1], b[2], b[3], b[4]);
				break;
			case UML:
				doUML(b[1], b[2], b[3], b[4]);
				break;
			case SDV:
				doSDV(b[1], b[2], b[3], b[4]);
				break;
			case UDV:
				doUDV(b[1], b[2], b[3], b[4]);
				break;
			case SMD:
				doSMD(b[1], b[2], b[3], b[4]);
				b += 4;
				continue;
			case UMD:
				doUMD(b[1], b[2], b[3], b[4]);
				break;
			case LSH:
				doLSH(b[1], b[2], b[3], b[4]);
				break;
			case RSH:
				doRSH(b[1], b[2], b[3], b[4]);
				break;
			case AND:
				doAND(b[1], b[2], b[3], b[4]);
				break;
			case OR:
				doOR(b[1], b[2], b[3], b[4]);
				break;
			case XOR:
				doXOR(b[1], b[2], b[3], b[4]);
				break;
			case EQ:
				doEQ(b[1], b[2], b[3], b[4]);
				break;
			case SLT:
				doSLT(b[1], b[2], b[3], b[4]);
				break;
			case ULT:
				doULT(b[1], b[2], b[3], b[4]);
				break;
			case SGT:
				doSGT(b[1], b[2], b[3], b[4]);
				break;
			case UGT:
				doUGT(b[1], b[2], b[3], b[4]);
				break;
			case LNT:
				doLNT();
				break;
			case BNT:
				doBNT(b[1], b[2]);
				break;
			case MIN:
				doMIN(b[1], b[2]);
				break;
			default:
				abortDueToInvalidInstruction(*b);
			}
			b += mArgumentCounts[*b];
		}
	}

	static std::vector<unsigned char> fileToVector(const char* s)
	{
		std::fstream f(s, std::ios_base::in);
		f.seekg(0, f.end);
		unsigned long size = f.tellg();
		f.seekg(0, f.beg);
		std::vector<unsigned char> v(size);
		f.read(reinterpret_cast<char*>(v.data()), size);
		return v;
	}

	static void vectorToFile(const char* s,
	std::vector<unsigned char>::iterator it, std::vector<unsigned char>::iterator endIt)
	{
		std::fstream f(s, std::ios_base::out);
		f.write(reinterpret_cast<char*>(&*it), endIt - it);
	}
};
