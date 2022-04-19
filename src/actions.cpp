#include "Actions.h"
#include "DataFile.h"
#include <string>
#include <utility>

const pv::Action *pv::actionFromName(std::string name) {
  using pv::ACTIONS;
  auto iter = std::find_if(
      std::cbegin(ACTIONS), std::cend(ACTIONS),
      [&](const pv::Action *action) { return action->name() == name; });

  return iter == ACTIONS.cend() ? nullptr : *iter;
}
