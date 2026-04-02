//============================================================================
// Name        : coflash
// Author      : Andrey Osipov
// Version     : 0.1.0
// Web Site    : https://hamlab.net
// GitHub:     : https://github.com/unsi9ned
// Description : CoFlash adapter
//============================================================================

#include <iostream>
#include "coflash_token.h"

using namespace std;

//------------------------------------------------------------------------------
// Точка входа
//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	CoFlashToken token;
	CoFlashToken::showVersion();

	if(argc < 2)
	{
		CoFlashToken::execCoFlash(COFLASH_ORIGIN_APP);
		return EXIT_SUCCESS;
	}

	if(strncmp(argv[1], "-", 1) == 0 || strncmp(argv[1], "--", 2) == 0)
	{
		token.setKey(string(argv[1]));
		token.setType(CoFlashToken::OPTION);
	}
	else
	{
		token.setKey(string(argv[1]));
		token.setType(CoFlashToken::COMMAND);
	}

	for(int i = 2; i < argc; i++)
	{
		token.addArgument(string(argv[i]));
	}

	return token.exec();
}
