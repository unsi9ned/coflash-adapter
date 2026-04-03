#ifndef PYOCD_CMD_BUILDER_H_
#define PYOCD_CMD_BUILDER_H_

#include <string>
#include <vector>
#include <sstream>
#include <cstdint>

class CoFlashCommand;
class EraseOption;
class ResetOption;

using namespace std;

class PyOcdCommandBuilder
{
private:
	std::string m_option;
	std::string m_command;      // flash, erase, verify
	std::string m_target;
	std::string m_packPath;
	std::string m_firmwarePath;
	std::string m_probeId;

	uint32_t m_frequency;
	std::string m_protocol;
	std::string m_eraseMode;

public:
	PyOcdCommandBuilder();

	PyOcdCommandBuilder& withCommand(CoFlashCommand cmd);
	PyOcdCommandBuilder& withTarget(const string& cpu);
	PyOcdCommandBuilder& withPack(const string& packPath);
	PyOcdCommandBuilder& withFirmware(const string& firmwarePath);
	PyOcdCommandBuilder& withFrequency(uint32_t frequency);
	PyOcdCommandBuilder& withInterface(const string& iface);
	PyOcdCommandBuilder& withErase(EraseOption mode);

	// Построение команды
	std::string build() const;
	std::vector<std::string> buildArgs() const;

	// Сброс всех настроек
	void reset();
};

#endif /* PYOCD_CMD_BUILDER_H_ */
