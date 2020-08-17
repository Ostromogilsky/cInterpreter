#include "interpreter.h"
const double PI = 3.141592653589793;

double fDegToRad(double num)
{
	return num * PI / 180;
}

double fMinus(double x)
{
	return -x;
}

double fRemainder(double left, double right)
{
	int l = static_cast<int>(left);
	int r = static_cast<int>(right);
	if (l == left && r == right)
		return static_cast<double>(l % r);
	else
		throw cExeption(Exeption::NON_INTEGER);
}

double fSum(double left, double right)
{
	return left + right;
}

double fDifference(double left, double right)
{
	return left - right;
}

double fProduct(double left, double right)
{
	return left * right;
}

double fQuotient(double left, double right)
{
	if (right == 0.0)
		throw cExeption(Exeption::ZERO_DENOMINATOR);
	return left / right;
}

double fSin(double x)
{
	return sin(x);
}

double fCos(double x)
{
	return cos(x);
}

double fSqrt(double x)
{
	return sqrt(x);
}

string cExeption::description() const
{
	string descr;
	if (mKey == Exeption::OK)
		descr = "No exeptions.";
	else if (mKey == Exeption::UNKNOWN_COMMAND)
		descr = "Unknown input command \"" + mDetails + "\"!";
	else if (mKey == Exeption::UNPAIRED_BRACKETS)
		descr = "Unpaired bracksets!";
	else if (mKey == Exeption::EMPTY_INPUT)
		descr = "Empty input string!";
	else if (mKey == Exeption::NON_CORRECT_ARGUMENTS)
		descr = "Number of argumements in command is not correct!";//"Number of argumements in command \"" + mDetails + "\" is not correct!";
	else if (mKey == Exeption::WRONG_SINTAXIS)
		descr = "Wrong sintaxis at " + mDetails + " position in input string!";
	else if (mKey == Exeption::EMPTY_BRACKETS)
		descr = "Empty brackets at " + mDetails + " position in string!";
	else if (mKey == Exeption::ZERO_DENOMINATOR)
		descr = "Zero in denominator!";
	else if (mKey == Exeption::NON_INTEGER)
		descr = "% gets integer arguments!";
	else
		descr = "Unknown exeption!";

	return descr;
}



void cInterpreter::addBrackets(string openingName, string closingName)
{
	cBracket* opening = new cBracket(openingName, cBracket::OPENING);
	mCommands.insert(std::make_pair(openingName, opening));
	cBracket* closing = new cBracket(closingName, cBracket::CLOSING);
	mCommands.insert(std::make_pair(closingName, closing));
}



void cInterpreter::addUnaryOperation(string name, doubleFunc1 func1, Priority priority, bool isPrefix)
{
	cUnaryOperation* to_add = new cUnaryOperation(name, func1, priority, isPrefix);
	mCommands.insert(std::make_pair(name, to_add));
}



void cInterpreter::addBinaryOperation(string name, doubleFunc2 func2, Priority priority)
{
	cBinaryOperation* to_add = new cBinaryOperation(name, func2, priority);
	mCommands.insert(std::make_pair(name, to_add));
	return;
}

void cInterpreter::addDoubleConstant(string name, double value)
{
	cDoubleConstant* to_add = new cDoubleConstant(name, value);
	mCommands.insert(std::make_pair(name, to_add));
	return;
}

cInterpreter::cInterpreter()
{
	mCommands.clear();
	addBrackets("(", ")");
	addBrackets("[", "]");

	addBinaryOperation("+", fSum, Priority::LOW);
	addBinaryOperation("-", fDifference, Priority::LOW);
	addBinaryOperation("*", fProduct, Priority::MIDDLE);
	addBinaryOperation("/", fQuotient, Priority::MIDDLE);
	addBinaryOperation("%", fRemainder, Priority::MIDDLE);

	addUnaryOperation("sqrt", fSqrt, Priority::VERY_HIGH, cUnaryOperation::PREFIX);
	addUnaryOperation("sin", fSin  , Priority::VERY_HIGH, cUnaryOperation::PREFIX);
	addUnaryOperation("cos", fCos  , Priority::VERY_HIGH, cUnaryOperation::PREFIX);
	addUnaryOperation("-_u", fMinus, Priority::VERY_HIGH, cUnaryOperation::PREFIX);
	addUnaryOperation("deg", fDegToRad, Priority::VERY_HIGH, cUnaryOperation::POSTFIX);
	addDoubleConstant("pi", PI);
}


bool cInterpreter::hasCommand(string name, cCommand** op)
{
	std::map<string, cCommand*>::iterator temp = mCommands.find(name);
	bool hasCommand = (temp != mCommands.end());

	if(hasCommand)
		*op = temp->second;

	return hasCommand;
}




cState& cState::operator=(const cState& right)
{
	if (this == &right)
		return *this;

	mState = right.get();

	return *this;
}




CommandType cInterpreter::readCommand(string* curCmd, bool unarySignFlag)
{
	if (unarySignFlag)
		*curCmd += "_u";

	cCommand* op;
	if (hasCommand(*curCmd, &op))
	{
		mCommandsInput.push(op);
		curCmd->clear();
		return op->type();
	}
	else
		throw cExeption(Exeption::UNKNOWN_COMMAND, *curCmd);
}


CommandType cInterpreter::readNumber(string* curNum)
{
	cCommand* num = new cDoubleConstant(*curNum);
	mCommandsInput.push(num);
	curNum->clear();
	return CommandType::CONSTANT;
}


bool inNum(char c) //for real numbers
{
	return c >= '0' && c <= '9';
}

bool inWord(char c) //for commands and veriables
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_'; // You may use also numbers to the end. Example: log2, a_11
}


void cInterpreter::checkState(char c)
{
	if (c == ' ')
		mCurState.set(State::SPACE);
	else if (c == '.' || c == ',')
		mCurState.set(State::DECIMAL_PLACE_SIGN);
	else if (inNum(c))
		mCurState.set(State::NUMBER);
	else if (inWord(c))
		mCurState.set(State::WORD);

	else
	{
		cCommand* op;
		string bracket;
		bracket.clear();
		bracket += c;

		if (hasCommand(bracket, &op))
		{
			switch (op->type())
			{
				case CommandType::OPENING_BRACKET: mCurState.set(State::OPENING_BRACKET); 
					break;
				case CommandType::CLOSING_BRACKET:  mCurState.set(State::CLOSING_BRACKET);
					break;
				default: mCurState.set(State::SIGN);
					break;
			}
		}

		else
			mCurState.set(State::SIGN);
	}
}


void cInterpreter::translateInput()
{
	char c;
	string curNum, curWord, curSign, curBracket;
	CommandType lastRead = CommandType::NOTHING;
	const bool UNARY_SIGN = true;
	const bool BINARY_SIGN = false;
	bool signFlag;
	bool integerPart = false;

	for (unsigned int i = 0; i < mInput.size(); i++)
	{
		c = mInput[i];
		checkState(c);

		if (lastRead == CommandType::NOTHING || lastRead == CommandType::BINARY ||
			lastRead == CommandType::OPENING_BRACKET || lastRead == CommandType::PREFIX_UNARY)
			signFlag = UNARY_SIGN;
		else
			signFlag = BINARY_SIGN;

		switch (mCurState.get())
		{
			case State::OPENING_BRACKET:
			{
				switch (mPrevState.get())
				{
					case State::OPENING_BRACKET: lastRead = readCommand(&curBracket); curBracket += c;
						break;
					case State::CLOSING_BRACKET: // the same as NUMBER
					case State::NUMBER: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i)); 
						break;
					case State::DECIMAL_PLACE_SIGN: mPrevState.set(State::SIGN);
					case State::SIGN: lastRead = readCommand(&curSign, signFlag); curBracket += c;
						break; 
					case State::WORD: lastRead = readCommand(&curWord); curBracket += c; 
						break; 
					case State::SPACE:
					{
						switch (lastRead)
						{
							case CommandType::OPENING_BRACKET: curBracket += c;
								break;
							case CommandType::CLOSING_BRACKET: // the same as VARIABLE
							case CommandType::VARIABLE: //the same as CONSTANT
							case CommandType::CONSTANT:  //the same as POSTFIX_UNARY
							case CommandType::POSTFIX_UNARY: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
								break;
							default: curBracket += c;
								break;
						}
						break;
					}
				}
				break;
			}

			case State::CLOSING_BRACKET:
			{
				switch (mPrevState.get())
				{
					case State::OPENING_BRACKET: throw cExeption(Exeption::EMPTY_BRACKETS, std::to_string(i));
						break;
					case State::CLOSING_BRACKET: lastRead = readCommand(&curBracket); curBracket += c; 
						break;
					case State::NUMBER: lastRead = readNumber(&curNum); curBracket += c; 
						break;
					case State::DECIMAL_PLACE_SIGN: throw cExeption(Exeption::NON_CORRECT_ARGUMENTS, curSign);
						break;
					case State::SIGN: throw cExeption(Exeption::NON_CORRECT_ARGUMENTS, curSign);
						break;
					case State::WORD: lastRead = readCommand(&curWord); curBracket += c; 
						break;
					case State::SPACE: //{curBracket += c; break; }
					{
						switch (lastRead)
						{
							case CommandType::OPENING_BRACKET: throw cExeption(Exeption::EMPTY_BRACKETS, std::to_string(i));
								break;
							case CommandType::PREFIX_UNARY: throw cExeption(Exeption::NON_CORRECT_ARGUMENTS, std::to_string(i));
								break;
							case CommandType::BINARY: throw cExeption(Exeption::NON_CORRECT_ARGUMENTS, std::to_string(i));
								break;
							case CommandType::NOTHING: throw cExeption(Exeption::UNPAIRED_BRACKETS);
								break;
							default: curBracket += c;
								break;
						}
						break;
					}
				}
				break;
			}

			case State::NUMBER:
			{
				switch (mPrevState.get())
				{
					case State::OPENING_BRACKET: integerPart = true; lastRead = readCommand(&curBracket); curNum += c;
						break;
					case State::CLOSING_BRACKET: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
						break;
					case State::NUMBER: curNum += c;
						break;
					case State::DECIMAL_PLACE_SIGN: integerPart = false; curNum += c;
						break;
					case State::SIGN: integerPart = true; lastRead = readCommand(&curSign, signFlag); curNum += c;
						break;
					case State::WORD: mCurState.set(State::WORD); curWord += c;
						break;
					case State::SPACE:
					{
						integerPart = true;
						switch (lastRead)
						{
							case CommandType::CLOSING_BRACKET:
							case CommandType::VARIABLE:
							case CommandType::CONSTANT:
							case CommandType::POSTFIX_UNARY: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
								break;
							default: curNum += c;
								break;
						}
						break;
					}
				}
				break;
			}

			case State::SIGN:
			{
				switch (mPrevState.get())
				{
					case State::OPENING_BRACKET: //the same as CLOSING_BRACKET
					case State::CLOSING_BRACKET: lastRead = readCommand(&curBracket); curSign += c; 
						break;
					case State::NUMBER: lastRead = readNumber(&curNum); curSign += c;
						break;
					case State::DECIMAL_PLACE_SIGN:
					case State::SIGN: curSign += c;
						break;
					case State::WORD: lastRead = readCommand(&curWord); curSign += c;
						break;
					case State::SPACE: curSign += c; 
						break;
				}
				break;
			}

			case State::WORD:
			{
				switch (mPrevState.get())
				{
					case State::OPENING_BRACKET: //the same as CLOSING_BRACKET
					case State::CLOSING_BRACKET: lastRead = readCommand(&curBracket); curWord += c;
						break;
					case State::NUMBER:  throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
						break;
					case State::DECIMAL_PLACE_SIGN:
					case State::SIGN: lastRead = readCommand(&curSign, signFlag); curWord += c;
						break;
					case State::WORD: curWord += c;
						break;
					case State::SPACE:
					{
						switch (lastRead)
						{
							case CommandType::CLOSING_BRACKET: 
							case CommandType::POSTFIX_UNARY: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
								break;
							default: curWord += c;
								break;
						}
						break;
					}
				}
				break;
			}

			case State::SPACE:
			{
				switch (mPrevState.get())
				{
					case State::OPENING_BRACKET:
					case State::CLOSING_BRACKET: lastRead = readCommand(&curBracket);
						break;
					case State::NUMBER: lastRead = readNumber(&curNum);
						break;
					case State::DECIMAL_PLACE_SIGN: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
						break;
					case State::SIGN: lastRead = readCommand(&curSign, signFlag);
						break;
					case State::WORD: lastRead = readCommand(&curWord);
						break;
				}
				break;
			}

			case State::DECIMAL_PLACE_SIGN:
			{
				switch (mPrevState.get())
				{
					case State::OPENING_BRACKET: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
						break;
					case State::NUMBER: if (integerPart) curNum += '.'; else throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
						break;
					case State::CLOSING_BRACKET: lastRead = readCommand(&curBracket); mCurState.set(State::SIGN); curSign += c;
						break;
					case State::DECIMAL_PLACE_SIGN: throw cExeption(Exeption::WRONG_SINTAXIS, std::to_string(i));
						break;
					case State::SIGN: mCurState.set(State::SIGN); curSign += c;
						break;
					case State::WORD:  mCurState.set(State::SIGN); lastRead = readCommand(&curWord); curSign += c;
						break;
					case State::SPACE: mCurState.set(State::SIGN);  curSign += c;
						break;
				}
				break;
			}
		}

		if (i == mInput.size() - 1)
		{
			switch (mCurState.get())
			{
				case State::OPENING_BRACKET: throw cExeption(Exeption::UNPAIRED_BRACKETS);
					break;
				case State::CLOSING_BRACKET: lastRead = readCommand(&curBracket);
					break;
				case State::NUMBER: lastRead = readNumber(&curNum);
					break;
				case State::DECIMAL_PLACE_SIGN: mPrevState.set(State::SIGN);
				case State::SIGN: lastRead = readCommand(&curSign, signFlag);
					break;
				case State::WORD: lastRead = readCommand(&curWord);
					break;
			}
		}

		mPrevState = mCurState;
	}
}



void cInterpreter::toRPN()
{
	cCommand* cur;
	std::stack<cCommand*> stack;
	while(!mCommandsInput.empty())
	{
		cur = mCommandsInput.front();
		switch (cur->type())
		{
			case CommandType::CONSTANT:
			case CommandType::VARIABLE:
			case CommandType::POSTFIX_UNARY: mRPN.push(cur); mCommandsInput.pop();
				break;
			case CommandType::PREFIX_UNARY:
			case CommandType::OPENING_BRACKET:  stack.push(cur); mCommandsInput.pop();
				break;
			case CommandType::CLOSING_BRACKET:
			{
				if (stack.empty())
					throw cExeption(Exeption::UNPAIRED_BRACKETS);
				while(stack.top()->type() != CommandType::OPENING_BRACKET)
				{
					mRPN.push(stack.top());
					stack.pop();

					if (stack.empty())
						throw cExeption(Exeption::UNPAIRED_BRACKETS);
				}
				if(static_cast<cBracket*> (stack.top())->pairID() != static_cast<cBracket*>(cur)->pairID())
					throw cExeption(Exeption::UNPAIRED_BRACKETS);
				stack.pop();
				mCommandsInput.pop();
				break;
			}
			case CommandType::BINARY:
			{
				while(!stack.empty() && ((stack.top()->type() == CommandType::PREFIX_UNARY || 
					stack.top()->type() == CommandType::BINARY) && 
					static_cast<cBinaryOperation*>(stack.top())->priority() >= static_cast<cBinaryOperation*> (cur)->priority()))
				{
					mRPN.push(stack.top());
					stack.pop();
				}
				stack.push(cur);
				mCommandsInput.pop();
				break;
			}
		}
	}

	while(!stack.empty())
	{
		switch(stack.top()->type())
		{
			case CommandType::BINARY:
			case CommandType::POSTFIX_UNARY:
			case CommandType::PREFIX_UNARY: mRPN.push(stack.top()); stack.pop();
				break;
			default: throw cExeption(Exeption::UNPAIRED_BRACKETS);
		}
	}
}



double cInterpreter::calculate()
{
	if (mRPN.empty())
		throw cExeption(Exeption::EMPTY_INPUT);

	std::stack<double> stack;
	while (!mRPN.empty())
	{
		switch (mRPN.front()->type())
		{
			case CommandType::VARIABLE:
			case CommandType::CONSTANT:
			{
				stack.push( static_cast <cDoubleConstant*> (mRPN.front())->value());
				mRPN.pop();
				break;
			}

			case CommandType::POSTFIX_UNARY:
			case CommandType::PREFIX_UNARY:
			{
				if (stack.empty())
					throw cExeption(Exeption::NON_CORRECT_ARGUMENTS, mRPN.front()->name());
				stack.top() = static_cast <cUnaryOperation*>(mRPN.front())->value(stack.top());
				mRPN.pop();
				break;
			}

			case CommandType::BINARY:
			{
				if (stack.empty())
					throw cExeption(Exeption::NON_CORRECT_ARGUMENTS);

				double right = stack.top();
				stack.pop();

				if (stack.empty())
					throw cExeption(Exeption::NON_CORRECT_ARGUMENTS);
				double left = stack.top();
				stack.top() = dynamic_cast <cBinaryOperation*>(mRPN.front())->value(left, right);
				mRPN.pop();
				break;
			}
		}
	}
	if(stack.size() != 1)
		throw cExeption(Exeption::NON_CORRECT_ARGUMENTS);
	return stack.top();
}