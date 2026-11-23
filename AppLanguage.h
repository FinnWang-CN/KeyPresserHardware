#pragma once

namespace AppLanguage {
constexpr int restartExitCode = 42;

void initialize();
bool isEnglish();
void setPreferredEnglish(bool english);
}
