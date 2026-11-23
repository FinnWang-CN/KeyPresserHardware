#ifndef ARDUINOCONTROLLER_HPP
#define ARDUINOCONTROLLER_HPP

#include <string>
#include <windows.h>
#include <vector>
#include <QSerialPortInfo>
#include <QRegularExpression>
#include <QDebug>
#include <QCoreApplication>

// Serial port communication class
class SerialPort {
private:
    HANDLE hSerial;
    DCB dcbSerialParams;
    COMMTIMEOUTS timeouts;

public:
    SerialPort() : hSerial(INVALID_HANDLE_VALUE), portName("") {}

    ~SerialPort() {
        close();
    }

    void close() {
        if (isOpen()) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
        }
        portName.clear();
    }

    static std::string findArduinoLeonardoPort() {
        for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
            const auto vendor = port.vendorIdentifier();
            if ((port.hasVendorIdentifier() && port.hasProductIdentifier() &&
                 (vendor == 0x2341 || vendor == 0x2A03) && port.productIdentifier() == 0x8036) ||
                port.description().contains(QStringLiteral("Leonardo"), Qt::CaseInsensitive)) {
                return port.portName().toStdString();
            }
        }
        return {};
    }

    bool open(const std::string& portName, DWORD baudRate) {
        const QString name = QString::fromStdString(portName).trimmed().toUpper();
        static const QRegularExpression comPort(QStringLiteral("^COM[1-9][0-9]*$"));
        close();
        if (!comPort.match(name).hasMatch())
            return false;

        const auto path = (QStringLiteral("\\\\.\\") + name).toStdString();
        hSerial = CreateFileA(
            path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if (hSerial == INVALID_HANDLE_VALUE) {
            return false;
        }

        // Configure serial port parameters
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return false;
        }

        dcbSerialParams.BaudRate = baudRate;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return false;
        }

        // Set timeouts
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(hSerial, &timeouts)) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return false;
        }

        this->portName = name.toStdString();
        return true;
    }

    bool isOpen() const {
        return hSerial != INVALID_HANDLE_VALUE;
    }

    bool write(const std::string& data) {
        if (!isOpen()) {
            return false;
        }

        DWORD bytesWritten;
        if (!WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, NULL)) {
            return false;
        }

        return bytesWritten == data.length();
    }

    bool read(std::string& data, DWORD maxBytes) {
        if (!isOpen()) {
            return false;
        }

        char buffer[1024];
        DWORD bytesRead;
        DWORD bufferSize = static_cast<DWORD>(sizeof(buffer));

        if (!ReadFile(hSerial, buffer, qMin(maxBytes, bufferSize), &bytesRead, NULL)) {
            return false;
        }

        data.assign(buffer, bytesRead);
        return true;
    }

    // Get the name of the currently open port
    std::string getPortName() const {
        if(portName.empty()){
            return SerialPort::findArduinoLeonardoPort();
        }
        return portName;
    }

private:
    std::string portName; // Store the name of the opened port
};

// Command type enumeration
enum class CommandType {
    PRESS_KEY = 0,
    RELEASE_KEY,
    TYPE_STRING,
    PRESS_COMBINATION,
    DELAY,
    MOUSE_MOVE,
    MOUSE_PRESS,
    MOUSE_RELEASE,
    MOUSE_CLICK,
    MOUSE_WHEEL
};

#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)

// Arduino controller class
class ArduinoController {
private:
    SerialPort serialPort;

public:
    bool connect(const std::string& portName, DWORD baudRate = 9600) {
        return serialPort.open(portName, baudRate);
    }

    void disconnect() {
        serialPort.close();
    }

    bool isConnected() const {
        return serialPort.isOpen();
    }

    // Send key press command
    bool pressKey(const std::string& key) {
        std::string command = "<0," + key + ">";
        return serialPort.write(command);
    }

    // Send key release command
    bool releaseKey(const std::string& key) {
        std::string command = "<1," + key + ">";
        return serialPort.write(command);
    }

    // Send string input command
    bool typeString(const std::string& str) {
        std::string command = "<2," + str + ">";
        return serialPort.write(command);
    }

    // Send key combination command
    bool pressKeyCombination(const std::vector<std::string>& keys) {
        std::string keysStr;
        for (size_t i = 0; i < keys.size(); ++i) {
            keysStr += keys[i];
            if (i < keys.size() - 1) {
                keysStr += ",";
            }
        }
        std::string command = "<3," + keysStr + ">";
        return serialPort.write(command);
    }

    // Send delay command
    bool delay(unsigned int milliseconds) {
        std::string command = "<4," + std::to_string(milliseconds) + ">";
        return serialPort.write(command);
    }

    // Send complete key operation (press then release)
    bool sendKey(const std::string& key, unsigned int pressDuration = 100) {
        if (!pressKey(key)) {
            return false;
        }

        // Wait for key press duration
        ::Sleep(pressDuration);

        return releaseKey(key);
    }

    // Send mouse move command
    bool mouseMove(int dx, int dy) {
        std::string command = "<" + std::to_string(static_cast<int>(CommandType::MOUSE_MOVE)) + "," +
                              std::to_string(dx) + "," + std::to_string(dy) + ">";
        return serialPort.write(command);
    }

    // Send mouse press command
    bool mousePress(int button) {
        std::string command = "<" + std::to_string(static_cast<int>(CommandType::MOUSE_PRESS)) + "," +
                              std::to_string(button) + ">";
        return serialPort.write(command);
    }

    // Send mouse release command
    bool mouseRelease(int button) {
        std::string command = "<" + std::to_string(static_cast<int>(CommandType::MOUSE_RELEASE)) + "," +
                              std::to_string(button) + ">";
        return serialPort.write(command);
    }

    // Send mouse click command
    bool mouseClick(int button, int clickCount = 1) {
        std::string command = "<" + std::to_string(static_cast<int>(CommandType::MOUSE_CLICK)) + "," +
                              std::to_string(button) + "," + std::to_string(clickCount) + ">";
        return serialPort.write(command);
    }

    // Send mouse wheel command
    bool mouseWheel(int delta) {
        std::string command = "<" + std::to_string(static_cast<int>(CommandType::MOUSE_WHEEL)) + "," +
                              std::to_string(delta) + ">";
        return serialPort.write(command);
    }

    // Upload sketch (HEX file) to Arduino board
    // Parameters:
    // - hexFilePath: Path to the HEX file
    // - avrdudePath: Path to avrdude executable (default: common Arduino IDE path)
    // - boardType: Arduino board type (default: "leonardo" for Arduino Leonardo)
    // - baudRate: Baud rate for uploading (default: 57600 for Arduino Leonardo)
    // - enableDebug: Print debug information
    bool uploadSketch(const std::string& hexFilePath,
                      bool enableDebug = true) {

        std::string arduino_cli_path = (QCoreApplication::applicationDirPath() + "/arduino-cli.exe").toStdString();
        //  command
        // 使用 --input-file 参数直接指定 HEX 文件
        std::string command = "cmd.exe /S /C \"" + arduino_cli_path + "\" upload --fqbn arduino:avr:leonardo --port " + serialPort.getPortName() + " --input-file \"" + hexFilePath + "\"";
        if (enableDebug) {
            qInfo() << "=== Starting sketch upload ===";
            qInfo() << "AVRDUDE command: " << command.c_str();
        }

        // Execute avrdude command
        int result = system(command.c_str());

        if (enableDebug) {
            if (result == 0) {
                qInfo() << "✅ Sketch upload completed successfully!";
            } else {
                qDebug() << "❌ Sketch upload failed with error code: " << result;
            }
            qInfo() << "=== Upload process finished ===";
        }

        return (result == 0);
    }
};

#endif // ARDUINOCONTROLLER_HPP
