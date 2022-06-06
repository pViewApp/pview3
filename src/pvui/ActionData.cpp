#include "ActionData.h"
#include <QObject>
#include <map>

namespace {

std::unordered_map<QString, const pvui::ActionData&> createNameMap() {
  std::unordered_map<QString, const pvui::ActionData&> map;

  for (const auto& data : pvui::actionData()) {
    map.insert({data.name, data});
  }

  return map;
}

std::unordered_map<pv::Action, const pvui::ActionData&> createActionMap() {
  std::unordered_map<pv::Action, const pvui::ActionData&> map;

  for (const auto& data : pvui::actionData()) {
    map.insert({data.action, data});
  }

  return map;
}

} // namespace

namespace pvui {

const std::vector<ActionData>& actionData() {
  static std::vector<ActionData> data = {{
                                             pv::Action::BUY,
                                             QObject::tr("Buy"),
                                         },
                                         {
                                             pv::Action::SELL,
                                             QObject::tr("Sell"),
                                         },
                                         {
                                             pv::Action::DEPOSIT,
                                             QObject::tr("In"),
                                         },
                                         {
                                             pv::Action::WITHDRAW,
                                             QObject::tr("Out"),
                                         },
                                         {
                                             pv::Action::DIVIDEND,
                                             QObject::tr("Dividend"),
                                         }};
  return data;
}

const ActionData* actionData(QString name) {
  static std::unordered_map map = createNameMap();
  const auto iter = map.find(name);
  return iter == map.end() ? nullptr : &iter->second;
}

const ActionData* actionData(pv::Action action) {
  static std::unordered_map map = createActionMap();
  const auto iter = map.find(action);
  return iter == map.end() ? nullptr : &iter->second;
}

} // namespace pvui
