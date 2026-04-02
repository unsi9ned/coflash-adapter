#ifndef COFLASH_TOKEN_H_
#define COFLASH_TOKEN_H_

#include <iostream>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include "version.h"

#define COFLASH_ORIGIN_APP     "coflash_origin.exe"
#define DEBUG_ENABLE           0

using namespace std;

//------------------------------------------------------------------------------
// Аргумент команды --driver=
//------------------------------------------------------------------------------
class DriverOption
{
private:
	string m_path;

public:
	DriverOption(const string& path = string())
	{
		m_path = path;
	}

	string path() const
	{
		return m_path;
	}

	string extension() const
	{
		string ext;

		if(!m_path.empty())
		{
			size_t dotPos = m_path.find_last_of('.');

			if(dotPos != string::npos)
			{
				ext = m_path.substr(dotPos + 1);
			}
		}

		transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		return ext;
	}

	bool isEmpty()
	{
		return m_path.empty();
	}
};

//------------------------------------------------------------------------------
// Аргумент команды --reset=
//------------------------------------------------------------------------------
class ResetOption
{
public:
	enum ResetMode
	{
		HW,
		VECTRESET,
		SYSRESETREQ
	};

private:
	ResetMode m_mode;

public:
	ResetOption() : m_mode(SYSRESETREQ) {}
	ResetOption(const string& mode)
	{
		string m = mode;
		transform(m.begin(), m.end(), m.begin(), ::toupper);

		if(m.compare("HW") == 0)
			m_mode = HW;
		else if(m.compare("VECTRESET") == 0)
			m_mode = VECTRESET;
		else
			m_mode = SYSRESETREQ;
	}

	string toString() const
	{
		string mode = "SYSRESETREQ";

		switch(m_mode)
		{
			case HW: mode = "HW"; break;
			case VECTRESET: mode = "VECTRESET"; break;
			case SYSRESETREQ: mode = "SYSRESETREQ"; break;
			default: mode = ""; break;
		}

		return mode;
	}
};

//------------------------------------------------------------------------------
// Аргумент команды --erase=
//------------------------------------------------------------------------------
class EraseOption
{
public:
	enum EraseVariant
	{
		Erase_All,
		Erase_Affected,
		Erase_Nothing
	};
private:
	EraseVariant m_eraseVariant;
public:
	EraseOption(EraseVariant variant = Erase_All) : m_eraseVariant(variant) {}

	EraseOption(const string& variant)
	{
		string v = variant;
		transform(v.begin(), v.end(), v.begin(), ::tolower);

		if(v.compare("affected") == 0)
			m_eraseVariant = Erase_Affected;
		else if(v.compare("nothing") == 0)
			m_eraseVariant = Erase_Nothing;
		else if(v.compare("all") == 0)
			m_eraseVariant = Erase_All;
		else
			m_eraseVariant = Erase_All;
	}

	string toString() const
	{
		string variant = "all";

		switch(m_eraseVariant)
		{
			case Erase_All: variant = "all"; break;
			case Erase_Affected: variant = "affected"; break;
			case Erase_Nothing: variant = "nothing"; break;
			default: variant = "all"; break;
		}

		return variant;
	}
};

//------------------------------------------------------------------------------
// Команда/опция CoFlash
//------------------------------------------------------------------------------
class CoFlashToken
{
public:
	enum TokenType
	{
		INVALID,
		COMMAND,
		OPTION
	};

private:
	TokenType m_type;
	string m_key;
	vector<string> m_args;

public:
	CoFlashToken(TokenType t = INVALID) : m_type(t) {}

	//
	// Сеттеры
	//
	void setType(TokenType t) { m_type = t; }
	void setKey(const string& key)
	{
		string k = key;
		transform(k.begin(), k.end(), k.begin(), ::tolower);
		m_key = k;
	}
	void addArgument(const string& arg) { m_args.push_back(arg); }

	//
	// Геттеры
	//
	TokenType getType() { return m_type; }
	string getKey() const { return m_key; }
	vector<string> getArguments() const { return m_args; }
	bool isCommand() { return m_type == COMMAND; }
	bool isOption() { return m_type == OPTION; }
	bool isValid() {return m_type != INVALID && !m_key.empty(); }

	//
	// Выполнение команды
	//
	int exec()
	{
		if(!isValid()) return EXIT_FAILURE;

		if(m_type == OPTION)
			return execOption();
		else
			return execCommand();
	}

	//
	// Выполнить консольную команду, адресованную оригинальному coflash
	//
	static void execCoFlash(const char * cmd)
	{
		FILE * stdOutput = popen(cmd, "rt");

		if(stdOutput)
		{
			char buffer[1024];

			while(fgets(buffer, sizeof(buffer), stdOutput) != NULL)
			{
				std::string line = buffer;
				std::cout << line;
			}
			pclose(stdOutput);
		}
	}

	//
	// Аргумент "драйвер"
	//
	DriverOption getDriver() const { return DriverOption(getValue("--driver=")); }

	//
	// Программируемый CPU
	//
	string getCpu() const
	{
		string cpu;

		if(m_type == COMMAND && !m_args.empty())
		{
			cpu = m_args.front();
		}

		return cpu;
	}

	//
	// Устройство-отладчик
	//
	string getDebugger() const { return getValue("--adapter-name="); }

	//
	// Частота работы отладчика
	//
	uint32_t getDebuggerClock() const
	{
		uint32_t freq = 0;
		string clk = getValue("--adapter-clk=");

		if(!clk.empty())
		{
			freq = stoul(clk);
		}

		return freq;
	}

	//
	// Отладочный интерфейс
	//
	string getDebugInterface() const { return getValue("--port="); }

	//
	// Режим сброса
	//
	ResetOption getResetMode() const { return ResetOption(getValue("--reset=")); }

	//
	// Опция стирания памяти
	//
	EraseOption getEraseOption() const { return EraseOption(getValue("--erase=")); }

	//
	// Делать проверку после программирования
	//
	bool verifyRequired() { return !(getValue("--verify=").compare("false") == 0); }

	//
	// Запускать после программирования
	//
	bool postRunRequired() { return !(getValue("--postrun=").compare("false") == 0); }

	//
	// Смещение в памяти
	//
	uint32_t getOffset() const
	{
		uint32_t offset = 0;
		string addr = getValue("--offset=");

		if(!addr.empty())
		{
			if(addr.find("0x") == 0 || addr.find("0X") == 0)
				offset = stoul(addr.substr(2), nullptr, 16);
			else
				offset = stoul(addr);
		}

		return offset;
	}

	//
	// Отобразить информацию о программе
	//
	static int showVersion()
	{
		cout << "CoFlash Adapter v" << APP_VERSION << endl;
		return EXIT_SUCCESS;
	}

private:

	//
	// Запросить опцию
	//
	int execOption()
	{
		if(m_key == "--adapter-version")
		{
			return showVersion();
		}
		else
		{
			execCoFlash(makeCmdLine().data());
		}

		return EXIT_SUCCESS;
	}

	//
	// Выполнить команду
	//
	int execCommand()
	{
#if DEBUG_ENABLE
		cout << getKey() << endl;
		cout << getDriver().path() << endl;
		cout << getDriver().extension() << endl;
		cout << getCpu() << endl;
		cout << getDebugger() << endl;
		cout << getDebuggerClock() << endl;
		cout << getDebugInterface() << endl;
		cout << getResetMode().toString() << endl;
		cout << getEraseOption().toString() << endl;
		cout << verifyRequired() << endl;
		cout << postRunRequired() << endl;
		cout << getOffset() << endl;
		cout << makeCmdLine() << endl;
#else
		execCoFlash(makeCmdLine().data());
		cout << "All operations:\t[Done]" << endl;
#endif
		return EXIT_SUCCESS;
	}

	//
	// Сформировать командную строку
	//
	string makeCmdLine()
	{
		string cmdLine = COFLASH_ORIGIN_APP;
		cmdLine += " " + m_key;

		for(size_t i = 0; i < m_args.size(); i++)
		{
			cmdLine += " " + m_args.at(i);
		}

		return cmdLine;
	}

	//
	// Получить путь к драйверу
	//
	string getValue(const string prefix) const
	{
		string value;

		for(size_t i = 0; i < m_args.size(); i++)
		{
			if(m_args[i].find(prefix) == 0)
			{
				value = m_args[i].substr(prefix.length());
				break;
			}
		}

		if(value.size() > 2 && value.front() == '"' && value.back() == '"')
		{
			value = value.substr(1, value.size() - 2);
		}

		return value;
	}
};

#endif /* COFLASH_TOKEN_H_ */
