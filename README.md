## CoFlash Adapter

*Version:* 0.1.0

**Описание**

_CoFlash Adapter_ - это программа-транслятор, заменяющий штатный coflash.exe в среде CoIDE. Она перехватывает команды от CoIDE и перенаправляет их либо в оригинальный _CoFlash_, либо в _pyOCD_, в зависимости от того, какая утилита поддерживает целевой микроконтроллер.

**Зачем это нужно?**

_CoIDE_ жёстко привязана к устаревшему _CoFlash_ и не позволяет выбрать другой инструмент программирования. _CoFlash Adapter_ решает эту проблему, имитируя работу _CoFlash_, но фактически передавая управление современным утилитам.

**Как это работает**
```batch
CoIDE → coflash.exe (адаптер) → анализ аргументов → выбор инструмента → CoFlash/pyOCD
```
Адаптер анализирует аргумент **--driver=<path_to_driver>** и выбирает утилиту в зависимости от расширения файла:
```batch
ELF → CoFlash
FLM → pyOCD 
```
**Пример трасляции команды**
```bash
# CoIDE вызывает команду программирования
coflash.exe program NRF52832_XXAA "nrf52.elf" --adapter-name=CMSIS-DAP --port=SWD --adapter-clk=2000000 --erase=affected --reset=SYSRESETREQ --driver="nrf52xxx.flm"

# CoFlash Adapter преобразует команду и вызывает pyOCD
pyocd flash --target NRF52832_XXAA --frequency 2000000 --erase sector "nrf52.elf"
```

**Требования**

- **MinGW-w64** (GCC для Windows)
- **make** (входит в состав MinGW)

**Сборка**
```bash
make
make clean
```

**Установка в CoIDE**
1. Перейти в каталог установки C:\CooCox\CoIDE
2. Скачать программу [**pyocd-windows-0.44.0.zip**](https://github.com/pyocd/pyOCD/releases/download/v0.44.0/pyocd-windows-0.44.0.zip)
3. Распаковать архив **pyocd-windows-0.44.0.zip** в каталог **bin** (C:\CooCox\CoIDE\bin)
4. Переименовать файл **coflash.exe** в **coflash_origin.exe** (C:\CooCox\CoIDE\bin\coflash_origin.exe)
5. Поместить файл **CoFlash Adapter (coflash.exe)** в папку **bin** (C:\CooCox\CoIDE\bin\coflash.exe)