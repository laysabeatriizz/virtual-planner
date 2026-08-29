#include "virtual_planner/api/json/reminder_json.hpp"

#include "virtual_planner/api/json/shared_json.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace virtual_planner::api::json {

namespace {

const nlohmann::json& required_field(const nlohmann::json& value,
                                     const char* field)
{
    if (!value.contains(field))
    {
        throw std::invalid_argument(
            std::string{"Reminder requires the field \""} + field + "\".");
    }

    return value.at(field);
}

std::uint64_t read_id(const nlohmann::json& value)
{
    const nlohmann::json& id = required_field(value, "id");

    if (!id.is_number_unsigned())
    {
        throw std::invalid_argument(
            "Reminder field \"id\" must be an unsigned integer.");
    }

    return id.get<std::uint64_t>();
}

std::string read_description(const nlohmann::json& value)
{
    const nlohmann::json& description = required_field(value, "description");

    if (!description.is_string())
    {
        throw std::invalid_argument(
            "Reminder field \"description\" must be a string.");
    }

    return description.get<std::string>();
}

} // namespace

nlohmann::json to_json(const domain::Reminder& reminder)
{
    return nlohmann::json{
        {"id", reminder.id()},
        {"description", reminder.description()},
        {"category", to_json(reminder.category())},
        {"date", to_json(reminder.date())},
        {"time_slot", to_json(reminder.time_slot())},
        {"type", to_json(reminder.type())},
        {"recurrence", to_json(reminder.recurrence())},
    };
}

domain::Reminder reminder_from_json(const nlohmann::json& value)
{
    if (!value.is_object())
    {
        throw std::invalid_argument("Reminder must be a JSON object.");
    }

    return domain::Reminder{
        read_id(value),
        read_description(value),
        category_from_json(required_field(value, "category")),
        date_from_json(required_field(value, "date")),
        time_slot_from_json(required_field(value, "time_slot")),
        reminder_type_from_json(required_field(value, "type")),
        reminder_recurrence_from_json(required_field(value, "recurrence")),
    };
}

} // namespace virtual_planner::api::json
