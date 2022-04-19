#ifndef PVUI_UTIL_SECURITYUTILS_H
#define PVUI_UTIL_SECURITYUTILS_H

#include <QValidator>
#include <QWidget>

namespace pvui {
namespace util {
class SecuritySymbolValidator : public QValidator {
  Q_OBJECT
public:
  QValidator::State validate(QString &input, int &pos) const override;
};
} // namespace util
} // namespace pvui

#endif // PVUI_UTIL_SECURITYUTILS_H
