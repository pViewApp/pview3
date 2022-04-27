#ifndef PVUI_ACTION_MAPPINGS_H
#define PVUI_ACTION_MAPPINGS_H

#include "pv/Action.h"
#include "pv/Actions.h"
#include <map>
#include <string>
#include <vector>

namespace {

static std::vector<std::pair<std::string, const pv::Action*>> mappingsBase = {{"Buy", &pv::actions::BUY},
                                                                              {"Sell", &pv::actions::SELL}};

std::map<std::string, const pv::Action*> generateNameToActionMappings() noexcept {
  std::map<std::string, const pv::Action*> output;
  for (const auto& pair : mappingsBase) {
    output.insert(pair);
  }
  return output;
}

std::map<const pv::Action*, std::string> generateActionToNameMappings() noexcept {
  std::map<const pv::Action*, std::string> output;
  for (const auto& pair : mappingsBase) {
    output.insert({pair.second, pair.first});
  }

  return output;
}

std::vector<const pv::Action*> generateActionsVector() noexcept {
  std::vector<const pv::Action*> output;
  for (const auto& pair : mappingsBase) {
    output.push_back(pair.second);
  }

  return output;
}

} // namespace

namespace pvui::actionmappings {

const std::map<std::string, const pv::Action*> nameToActionMappings = generateNameToActionMappings();
const std::map<const pv::Action*, std::string> actionToNameMappings = generateActionToNameMappings();
const std::vector<const pv::Action*> actions = generateActionsVector();

} // namespace pvui::actionmappings

#endif // PVUI_ACTION_MAPPINGS_H
