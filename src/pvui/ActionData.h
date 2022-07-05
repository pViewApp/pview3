#ifndef PVUI_ACTION_MAPPINGS_H
#define PVUI_ACTION_MAPPINGS_H

#include <QString>
#include <vector>
#include "pv/DataFile.h"

namespace pvui {

struct ActionData {
  pv::Action action;
  QString name; // Not translated, use tr() before displaying
};

const std::vector<ActionData>& actionData();

const ActionData* actionData(QString name);
const ActionData* actionData(pv::Action action);

} // namespace pvui

#endif // PVUI_ACTION_MAPPINGS_H
