#ifndef COFLASH_TOKEN_H_
#define COFLASH_TOKEN_H_

#include <windows.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include "version.h"
#include "pyocd_cmd_builder.h"

#define COFLASH_ORIGIN_APP     "coflash_origin.exe"
#define DEBUG_ENABLE           (1)

using namespace std;

//------------------------------------------------------------------------------
// Команды CoFlash
//------------------------------------------------------------------------------
class CoFlashCommand
{
public:
	enum Command
	{
		erase,            // Erase the flash device.
		program,          // Program a bin file into the flash device.
		verify,           // Verify  a bin file.
		blankcheck,       // Do blank check.
		help,             // Show help for the basic command that you specify.
	};

private:

	Command m_command;

public:
	CoFlashCommand(Command c) : m_command(c) {}
	CoFlashCommand(const string& c = "help")
	{
		string cmd = c;
		transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

		if(cmd == "erase") m_command = erase;
		else if(cmd == "program") m_command = program;
		else if(cmd == "verify") m_command = verify;
		else if(cmd == "blankcheck") m_command = blankcheck;
		else m_command = help;
	}

	string toString() const
	{
		string cmd;

		if(m_command == erase) cmd = "erase";
		else if(m_command == program) cmd = "program";
		else if(m_command == verify) cmd = "verify";
		else if(m_command == blankcheck) cmd = "blankcheck";
		else cmd = "help";

		return cmd;
	}

	Command getType() const { return m_command; }
};

//------------------------------------------------------------------------------
// Аргумент, представляющий собой путь к файлу
//------------------------------------------------------------------------------
class FileArgument
{
protected:
	string m_path;

public:
	FileArgument(const string& path = string())
	{
		setPath(path);
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

	string directory() const
	{
		return uplevelDir(m_path);
	}

	static string uplevelDir(string path, bool * rootLevel = nullptr)
	{
		string dir;

		if(!path.empty())
		{
			// Замена слешей на обратные
			replace(path.begin(), path.end(), '/', '\\');

			// Удаляем все после слэша
			size_t slashPos = path.find_last_of('\\');

			if(slashPos != string::npos)
			{
				dir = path.substr(0, slashPos);

				// Последний уровень
				if(dir.find_last_of('\\') == string::npos)
				{
					if(rootLevel) *rootLevel = true;
					dir += '\\';
				}
				else if(rootLevel)
					*rootLevel = false;
			}
		}

		return dir;
	}

	bool isEmpty()
	{
		return m_path.empty();
	}

	void setPath(const string& path)
	{
		string p = path;
		replace(p.begin(), p.end(), '/', '\\');
		m_path = p;
	}
};

//------------------------------------------------------------------------------
// Аргумент команды --driver=
//------------------------------------------------------------------------------
class DriverOption : public FileArgument
{
private:
	string m_path;

public:
	DriverOption(const string& path = string()) : FileArgument(path){}
};

//------------------------------------------------------------------------------
// Аргумент команды путь к прошивке
//------------------------------------------------------------------------------
class FirmwareOption : public FileArgument
{
private:
	string m_path;

public:
	FirmwareOption(const string& path = string()) : FileArgument(path){}
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

	EraseVariant getVariant() { return m_eraseVariant; }
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
		HANDLE hReadPipe, hWritePipe;
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

		// Создаём pipe для захвата вывода
		if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
		{
			std::cerr << "Failed to create pipe" << std::endl;
			return;
		}

		// Настраиваем STARTUPINFO
		STARTUPINFO si = { sizeof(STARTUPINFO) };
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = hWritePipe;
		si.hStdError = hWritePipe;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

		// Запускаем процесс
		PROCESS_INFORMATION pi;
		char* cmdLine = _strdup(cmd);

		BOOL success = CreateProcess(
			NULL, cmdLine, NULL, NULL, TRUE,
			CREATE_NO_WINDOW, NULL, NULL, &si, &pi
		);

		free(cmdLine);
		CloseHandle(hWritePipe);  // закрываем писатель в родителе

		if (!success)
		{
			std::cerr << "Failed to create process" << std::endl;
			CloseHandle(hReadPipe);
			return;
		}

		// Читаем вывод
		char buffer[4096];
		DWORD bytesRead;

		while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
		{
			// Просто выводим, но убираем лишние \r
			for (DWORD i = 0; i < bytesRead; ++i)
			{
				if (buffer[i] != '\r')
				{
					std::cout << buffer[i];
				}
			}

			std::flush(std::cout);
		}

		// Ждём завершения процесса
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Получаем код возврата
		DWORD exitCode = 0;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hReadPipe);
	}

	//
	// Возврат команды
	//
	CoFlashCommand getCommand() const { return CoFlashCommand(m_key); }

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
	// Прошивка
	//
	FirmwareOption getFirmware() const
	{
		FirmwareOption firmware;

		if(m_type == COMMAND && m_args.size() > 1)
		{
			firmware.setPath(m_args.at(1));
		}

		return firmware;
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
		cout << getFirmware().path() << endl;
		cout << getFirmware().extension() << endl;
		cout << getDriver().path() << endl;
		cout << getDriver().extension() << endl;
		cout << getDriver().directory() << endl;

		//cout << getDriver().uplevelDir(getDriver().directory()) << endl;
		string path = getDriver().directory();
		bool rootLevel = false;

		while(!path.empty() && !rootLevel)
		{
			cout << path << endl;
			path = DriverOption::uplevelDir(path, &rootLevel);
		}
		cout << path << endl;

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

		CoFlashCommand command = getCommand();

		if(getDriver().extension() == "flm" &&
		   (command.getType() == CoFlashCommand::program ||
		    command.getType() == CoFlashCommand::erase ||
		    command.getType() == CoFlashCommand::verify ||
			command.getType() == CoFlashCommand::help))
		{
			PyOcdCommandBuilder pyOcdBuilder;
			string cmd = pyOcdBuilder.
							withCommand(command).
							withTarget(getCpu()).
							withPack("cmsis.pack").
							withFirmware("firmware.elf").
							withFrequency(getDebuggerClock()).
							withErase(getEraseOption()).
							build();

			cout << cmd << endl;
		}
#else
		execCoFlash(makeCmdLine().data());
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
