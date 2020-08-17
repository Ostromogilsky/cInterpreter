#include "interpreter.h"
#include <iostream>
#include <string>
//GitHub test
//////  trigonometrical functions in radians!
using std::cout;
using std::cin;
using std::endl;
int cBracket::mIDgenerator = 1;

int main()
{
	std::string input;
	while (true)
	{
		std::getline(cin, input);
		if (input == "exit" || input == "ex")
				return 0;
		try
		{

			cInterpreter in;
			in.load(input);
			in.translateInput();
			in.toRPN();
			cout << "Answer: "<< in.calculate() << "\n";
		}

		catch (cExeption e)
		{
			cout << e.description() << "\n";
		}

	}
	return 0;
}

