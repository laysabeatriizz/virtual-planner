#pragma once

#include <nlohmann/json.hpp>

#include "virtual_planner/domain/entities/reminder.hpp"

namespace virtual_planner::api::json {

nlohmann::json to_json(const domain::Reminder& reminder);

domain::Reminder reminder_from_json(const nlohmann::json& value);

} // namespace virtual_planner::api::json
