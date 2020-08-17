#include <map>
#include <stack>
#include <queue>   
#include <string>
#include <vector>
#include <iostream>

using std::string;
typedef double (*doubleFunc1)(double);
typedef double (*doubleFunc2)(double, double);
#ifndef INTERPRETER
#define INTERPRETER


const enum class CommandType : int { OPENING_BRACKET, CLOSING_BRACKET, VARIABLE, CONSTANT, PREFIX_UNARY, POSTFIX_UNARY, BINARY, NOTHING };
const class cCommand //pure virtual
{
protected:
	string mName;
	CommandType mType;
	cCommand() { mName.clear(); mType = CommandType::NOTHING; }

public:
	cCommand(string name, CommandType type) { mName = name; mType = type; }
	virtual ~cCommand() = 0 {}

	string name() const { return mName; }
	CommandType const type() { return mType; }
};


class cBracket : public cCommand
{
public:
	static const bool OPENING = true;
	static const bool CLOSING = false;

private:
	int mPairID;
	static int mIDgenerator;

public:
	cBracket(string name, bool isOpening) : cCommand(name, isOpening ? CommandType::OPENING_BRACKET : CommandType::CLOSING_BRACKET)
	{	
		mPairID = mIDgenerator; if (!isOpening) mIDgenerator++;
	}

	int pairID() { return mPairID; }
};



class cDoubleConstant : public cCommand
{
private:
	double mValue;

public:
	cDoubleConstant(double value) : cCommand(std::to_string(value), CommandType::CONSTANT) { mValue = value; }
	cDoubleConstant(string value) : cCommand(value, CommandType::CONSTANT) { mValue = stod(value); }
	cDoubleConstant(string name, double value) : cCommand(name, CommandType::CONSTANT) { mValue = value; }

	double value() { return mValue; }
};


//low = {+, - ,...}; middle = {*, /, ...}; high = {^, ...}; very high for functions as sin, cos...
const enum class  Priority { LOW, MIDDLE, HIGH, VERY_HIGH };
class cPriority //pure virtual
{
protected:
	Priority mPriority;

public:
	cPriority(Priority priority) { mPriority = priority; }
	Priority priority() { return mPriority; }
	virtual ~cPriority() = 0 {}
};



class cUnaryOperation : public cCommand, public cPriority
{
public:
	static const bool POSTFIX = false;
	static const bool PREFIX = true;

private:
	doubleFunc1 mFunc;

public:
	cUnaryOperation(string name, doubleFunc1 func1, Priority priority, bool isPrefix) :
		 cCommand(name, isPrefix ? CommandType::PREFIX_UNARY : CommandType::POSTFIX_UNARY), cPriority(priority) 
		{ mFunc = func1;}

	double value(double arg) { return mFunc(arg); }
};



class cBinaryOperation : public cCommand, public cPriority
{
private:
	doubleFunc2 mFunc;

public:
	cBinaryOperation(string name, doubleFunc2 func, Priority priority) :
		cCommand(name, CommandType::BINARY), cPriority(priority) 
		{ mFunc = func; }

	double value(double left, double right) { return mFunc(left, right); }
};



class cDoubleVariable : public cCommand
{
private:
	double mValue;
public:
	cDoubleVariable(string name, double value = NULL) : cCommand(name, CommandType::VARIABLE) { mValue = value; }

	void put(double value) { mValue = value; }
	double value() { return mValue; }
};

const enum class Exeption { OK, UNKNOWN_COMMAND, UNPAIRED_BRACKETS, EMPTY_INPUT, NON_CORRECT_ARGUMENTS, WRONG_SINTAXIS, EMPTY_BRACKETS, ZERO_DENOMINATOR, NON_INTEGER };
class cExeption
{
private:
	Exeption mKey;
	string mDetails;
public:
	cExeption(Exeption key = Exeption::OK, string details = "") { mKey = key, mDetails = details; }
	string description() const;
};

//sign for binary delimiter operations
//word for other operations and variables
const enum class State { NUMBER, SPACE, WORD, SIGN, OPENING_BRACKET, CLOSING_BRACKET, DECIMAL_PLACE_SIGN };
class cState
{
private:
	State mState;

public:
	cState() { mState = State::SPACE; }
	cState& operator= (const cState& right);

	void set(State state) { mState = state; }
	State get() const { return mState; }

	bool isNumber() { return mState == State::NUMBER; }
	bool isSpace() { return mState == State::SPACE; }
	bool isWord() { return mState == State::WORD; }
	bool isSign() { return mState == State::SIGN; }
	bool isOpeningBracket() { return mState == State::OPENING_BRACKET; }
	bool isClosingBracket() { return mState == State::CLOSING_BRACKET; }
};


class cInterpreter
{
private:
	CommandType readCommand(string* curCmd, bool unarySignFlag = false);
	CommandType readNumber(string* curNum);
	bool hasCommand(string name, cCommand** op);

	cState mCurState, mPrevState;
	void checkState(char c);

	std::map<string, cCommand*> mCommands;

	string mInput;
	std::queue<cCommand*> mCommandsInput;

	std::queue<cCommand*> mRPN;

public:
	cInterpreter();
	void load(string input) { mInput = input; }

	void addBrackets(string openingName, string closingName);
	void addUnaryOperation(string name, doubleFunc1 func, Priority priority, bool isPrefix);
	void addBinaryOperation(string name, doubleFunc2 func, Priority priority);
	void addDoubleConstant(string name, double value);

	void printRPN()
	{
		std::cout << "\n";
		while (!mRPN.empty())
		{
			std::cout << "\"" << mRPN.front()->name() << "\" " << static_cast<int> (mRPN.front()->type()) << "\n";
			mRPN.pop();
		}
		std::cout << "\n";
	}
	string getInput() { return mInput; }

	void translateInput();
	void printCommandInput()
	{
		std::cout << "\n";
		while (!mCommandsInput.empty())
		{	std::cout << "\"" << mCommandsInput.front()->name() << "\" " << static_cast<int> (mCommandsInput.front()->type()) << "\n";
			mCommandsInput.pop();
		}
		std::cout << "\n";
	}
	void toRPN();

	double calculate();
};




#endif // INTERPRETER
