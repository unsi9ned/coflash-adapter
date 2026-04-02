#include "pyocd_cmd_builder.h"
#include "coflash_token.h"

using namespace std;

//------------------------------------------------------------------------------
// Сброс всех параметров
//------------------------------------------------------------------------------
PyOcdCommandBuilder::PyOcdCommandBuilder() : m_frequency(0)
{
	reset();
}

//------------------------------------------------------------------------------
// Создание команды
//------------------------------------------------------------------------------
std::string PyOcdCommandBuilder::build() const
{
	auto args = buildArgs();
	std::ostringstream oss;
	oss << "pyocd";
	for (const auto &arg : args)
	{
		oss << " " << arg;
	}
	return oss.str();
}

//------------------------------------------------------------------------------
// Создание аргументов
//------------------------------------------------------------------------------
std::vector<std::string> PyOcdCommandBuilder::buildArgs() const
{
	std::vector<std::string> args;

	if(!m_option.empty())
	{
		args.push_back(m_option);
		return args;
	}
	else if(m_command.empty())
	{
		args.push_back("-h");
		return args;
	}
	else
	{
		args.push_back(m_command);

		if(!m_eraseMode.empty() && m_command == "erase")
		{
			args.push_back("--" + m_eraseMode);
		}

		// Target
		if(!m_target.empty())
		{
			args.push_back("--target");
			args.push_back(m_target);
		}

		// Pack
		if(!m_packPath.empty())
		{
			args.push_back("--pack");
			args.push_back('"' + m_packPath + '"');
		}

		// Frequency
		if(m_frequency > 0)
		{
			args.push_back("--frequency");
			args.push_back(to_string(m_frequency));
		}

		// Режим стирания
		if(!m_eraseMode.empty() && m_command == "flash")
		{
			args.push_back("--erase");
			args.push_back(m_eraseMode);
		}

		// Firmware (в самом конце)
		if(!m_firmwarePath.empty() && m_command == "flash")
		{
			args.push_back('"' + m_firmwarePath + '"');
		}
	}

	return args;
}

//------------------------------------------------------------------------------
// Создание аргумента "Команда"
//------------------------------------------------------------------------------
PyOcdCommandBuilder& PyOcdCommandBuilder::withCommand(CoFlashCommand cmd)
{
	switch(cmd.getType())
	{
		case CoFlashCommand::erase:
			m_command = "erase";
		break;

		case CoFlashCommand::program:
			m_command = "flash";
		break;

		case CoFlashCommand::verify:
			m_option = "-h";
		break;

		case CoFlashCommand::help:
			m_option = "-h";
		break;
		default:
			m_option = "-h";
		break;
	}
	return *this;
}

//------------------------------------------------------------------------------
// Добавление имени CPU
//------------------------------------------------------------------------------
PyOcdCommandBuilder& PyOcdCommandBuilder::withTarget(const string& cpu)
{
	m_target = cpu;
	return *this;
}

//------------------------------------------------------------------------------
// Добавление пути к пакету CMSIS
//------------------------------------------------------------------------------
PyOcdCommandBuilder& PyOcdCommandBuilder::withPack(const string &packPath)
{
	m_packPath = packPath;
	return *this;
}

//------------------------------------------------------------------------------
// Добавление пути к прошивке
//------------------------------------------------------------------------------
PyOcdCommandBuilder& PyOcdCommandBuilder::withFirmware(const string &firmwarePath)
{
	m_firmwarePath = firmwarePath;
	return *this;
}

//------------------------------------------------------------------------------
// Частота отладчика
//------------------------------------------------------------------------------
PyOcdCommandBuilder& PyOcdCommandBuilder::withFrequency(uint32_t frequency)
{
	m_frequency = frequency;
	return *this;
}

//------------------------------------------------------------------------------
// Отладочный интерфейс
//------------------------------------------------------------------------------
PyOcdCommandBuilder& PyOcdCommandBuilder::withInterface(const string &iface)
{
	if(!iface.empty())
	{
		string wire = iface;
		transform(wire.begin(), wire.end(), wire.begin(), ::tolower);

		if(wire == "jtag")
			m_protocol = "Jtag";
		else
			m_protocol = "Swd";
	}
	return *this;
}

//------------------------------------------------------------------------------
// Режим стирания
//------------------------------------------------------------------------------
PyOcdCommandBuilder& PyOcdCommandBuilder::withErase(EraseOption mode)
{
	switch(mode.getVariant())
	{
		case EraseOption::Erase_All:
			m_eraseMode = "chip";
		break;

		case EraseOption::Erase_Affected:
			m_eraseMode = "sector";
		break;

		default:
			m_eraseMode = "auto";
		break;
	}

	return *this;
}

//------------------------------------------------------------------------------
// Сброс всех настроек
//------------------------------------------------------------------------------
void PyOcdCommandBuilder::reset()
{
	m_option = "";
	m_command = "";
	m_target.clear();
	m_packPath.clear();
	m_firmwarePath.clear();
	m_probeId.clear();
	m_frequency = 0;
	m_protocol = "";
	m_eraseMode = "";
}
