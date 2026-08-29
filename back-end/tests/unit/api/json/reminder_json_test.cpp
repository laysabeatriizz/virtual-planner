#include <nlohmann/json.hpp>

#include "virtual_planner/api/json/reminder_json.hpp"
#include "virtual_planner/shared/errors.hpp"
#include "support/expect.hpp"

#include <chrono>
#include <stdexcept>
#include <string>

using namespace virtual_planner;

namespace {

template <typename Callable>
bool throws_invalid_argument(Callable callable)
{
    try
    {
        callable();
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }
    catch (...)
    {
        return false;
    }

    return false;
}

template <typename Callable>
bool throws_domain_error(Callable callable)
{
    try
    {
        callable();
    }
    catch (const shared::DomainError&)
    {
        return true;
    }
    catch (...)
    {
        return false;
    }

    return false;
}

void expect_round_trip(const domain::Reminder& original,
                       const domain::Reminder& parsed,
                       const char* label)
{
    VP_EXPECT(parsed.id() == original.id(),
              std::string{label} + " deve preservar o id");
    VP_EXPECT(parsed.description() == original.description(),
              std::string{label} + " deve preservar a descrição");
    VP_EXPECT(parsed.category() == original.category(),
              std::string{label} + " deve preservar a categoria");
    VP_EXPECT(parsed.date() == original.date(),
              std::string{label} + " deve preservar a data");
    VP_EXPECT(parsed.time_slot().start() == original.time_slot().start(),
              std::string{label} + " deve preservar o início do horário");
    VP_EXPECT(parsed.time_slot().end() == original.time_slot().end(),
              std::string{label} + " deve preservar o fim do horário");
    VP_EXPECT(parsed.type() == original.type(),
              std::string{label} + " deve preservar o tipo");
    VP_EXPECT(parsed.recurrence() == original.recurrence(),
              std::string{label} + " deve preservar a recorrência");
}

} // namespace

int main()
{
    // Arrange
    const domain::Reminder once{
        42,
        "Revisar paradigmas de C++",
        domain::Category::Study,
        domain::Date{28, 8, 2026},
        domain::TimeSlot{std::chrono::hours{9}, std::chrono::hours{10}},
        domain::ReminderType::Study,
        domain::ReminderRecurrence::Once,
    };

    // Act
    const nlohmann::json serialized_once = api::json::to_json(once);

    // Assert
    VP_EXPECT(serialized_once.at("id") == 42,
              "o id do Reminder deve ser serializado");
    VP_EXPECT(serialized_once.at("description") == "Revisar paradigmas de C++",
              "a descrição do Reminder deve ser serializada");
    VP_EXPECT(serialized_once.at("category").is_string(),
              "a categoria deve ser uma string JSON");
    VP_EXPECT(serialized_once.at("category") == "Study",
              "a categoria deve usar a representação JSON compartilhada");
    VP_EXPECT(serialized_once.at("date").is_string(),
              "a data deve ser uma string JSON");
    VP_EXPECT(serialized_once.at("date") == "2026-08-28",
              "a data deve usar a representação ISO compartilhada");
    VP_EXPECT(serialized_once.at("time_slot").at("start").is_number_integer(),
              "o início deve ser um número inteiro de minutos");
    VP_EXPECT(serialized_once.at("time_slot").at("start") == 540,
              "o início deve ser serializado em minutos");
    VP_EXPECT(serialized_once.at("time_slot").at("end").is_number_integer(),
              "o fim deve ser um número inteiro de minutos");
    VP_EXPECT(serialized_once.at("time_slot").at("end") == 600,
              "o fim deve ser serializado em minutos");
    VP_EXPECT(serialized_once.contains("type"),
              "o tipo deve estar explícito no JSON");
    VP_EXPECT(serialized_once.at("type").is_string(),
              "o tipo deve ser uma string JSON");
    VP_EXPECT(serialized_once.at("type") == "Study",
              "o tipo deve usar a representação JSON compartilhada");
    VP_EXPECT(serialized_once.contains("recurrence"),
              "a recorrência Once deve estar explícita no JSON");
    VP_EXPECT(serialized_once.at("recurrence").is_string(),
              "a recorrência deve ser uma string JSON");
    VP_EXPECT(serialized_once.at("recurrence") == "Once",
              "a recorrência única deve ser serializada como Once");

    // Act
    const domain::Reminder parsed_once =
        api::json::reminder_from_json(serialized_once);

    // Assert
    expect_round_trip(once, parsed_once, "o round-trip do Reminder Once");

    // Arrange
    const domain::Reminder recurring{
        84,
        "Reunião semanal",
        domain::Category::Work,
        domain::Date{31, 8, 2026},
        domain::TimeSlot{std::chrono::hours{14}, std::chrono::hours{15}},
        domain::ReminderType::Meeting,
        domain::ReminderRecurrence::Weekly,
    };

    // Act
    const nlohmann::json serialized_recurring = api::json::to_json(recurring);
    const domain::Reminder parsed_recurring =
        api::json::reminder_from_json(serialized_recurring);

    // Assert
    VP_EXPECT(serialized_recurring.contains("type"),
              "o tipo do Reminder recorrente deve estar explícito");
    VP_EXPECT(serialized_recurring.at("type") == "Meeting",
              "o tipo recorrente deve usar a representação compartilhada");
    VP_EXPECT(serialized_recurring.contains("recurrence"),
              "a recorrência semanal deve estar explícita no JSON");
    VP_EXPECT(serialized_recurring.at("recurrence") == "Weekly",
              "a recorrência semanal deve ser serializada como Weekly");
    expect_round_trip(
        recurring, parsed_recurring, "o round-trip do Reminder recorrente");

    // Arrange
    nlohmann::json missing_category = serialized_once;
    missing_category.erase("category");
    nlohmann::json missing_recurrence = serialized_once;
    missing_recurrence.erase("recurrence");
    nlohmann::json negative_id = serialized_once;
    negative_id["id"] = -1;
    nlohmann::json fractional_id = serialized_once;
    fractional_id["id"] = 1.5;
    nlohmann::json string_id = serialized_once;
    string_id["id"] = "42";
    nlohmann::json invalid_description_type = serialized_once;
    invalid_description_type["description"] = 42;
    nlohmann::json invalid_category = serialized_once;
    invalid_category["category"] = "Unknown";
    nlohmann::json invalid_type = serialized_once;
    invalid_type["type"] = "Unknown";
    nlohmann::json invalid_recurrence = serialized_once;
    invalid_recurrence["recurrence"] = "Yearly";
    nlohmann::json invalid_date = serialized_once;
    invalid_date["date"] = "2026-02-30";
    nlohmann::json invalid_time_slot = serialized_once;
    invalid_time_slot["time_slot"] = {{"start", 600}, {"end", 540}};
    nlohmann::json empty_description = serialized_once;
    empty_description["description"] = "";
    nlohmann::json blank_description = serialized_once;
    blank_description["description"] = "   ";

    // Act and Assert
    VP_EXPECT(
        throws_invalid_argument(
            [] { return api::json::reminder_from_json(nlohmann::json::array()); }),
        "um Reminder que não seja objeto deve ser rejeitado");
    VP_EXPECT(
        throws_invalid_argument([&missing_category] {
            return api::json::reminder_from_json(missing_category);
        }),
        "um Reminder sem campo obrigatório deve ser rejeitado");
    VP_EXPECT(
        throws_invalid_argument([&missing_recurrence] {
            return api::json::reminder_from_json(missing_recurrence);
        }),
        "um Reminder sem recorrência deve ser rejeitado");
    VP_EXPECT(
        throws_invalid_argument([&negative_id] {
            return api::json::reminder_from_json(negative_id);
        }),
        "um id negativo deve ser rejeitado");
    VP_EXPECT(
        throws_invalid_argument([&fractional_id] {
            return api::json::reminder_from_json(fractional_id);
        }),
        "um id fracionário deve ser rejeitado");
    VP_EXPECT(
        throws_invalid_argument([&string_id] {
            return api::json::reminder_from_json(string_id);
        }),
        "um id em string deve ser rejeitado");
    VP_EXPECT(
        throws_invalid_argument([&invalid_description_type] {
            return api::json::reminder_from_json(invalid_description_type);
        }),
        "uma descrição que não seja string deve ser rejeitada");
    VP_EXPECT(
        throws_invalid_argument([&invalid_category] {
            return api::json::reminder_from_json(invalid_category);
        }),
        "uma categoria inválida deve ser rejeitada");
    VP_EXPECT(
        throws_invalid_argument([&invalid_type] {
            return api::json::reminder_from_json(invalid_type);
        }),
        "um tipo inválido deve ser rejeitado");
    VP_EXPECT(
        throws_invalid_argument([&invalid_recurrence] {
            return api::json::reminder_from_json(invalid_recurrence);
        }),
        "uma recorrência inválida deve ser rejeitada");
    VP_EXPECT(
        throws_invalid_argument([&invalid_date] {
            return api::json::reminder_from_json(invalid_date);
        }),
        "uma data inválida deve ser rejeitada");
    VP_EXPECT(
        throws_invalid_argument([&invalid_time_slot] {
            return api::json::reminder_from_json(invalid_time_slot);
        }),
        "um horário inválido deve ser rejeitado");
    VP_EXPECT(
        throws_domain_error([&empty_description] {
            return api::json::reminder_from_json(empty_description);
        }),
        "uma descrição vazia deve ser rejeitada pelo domínio");
    VP_EXPECT(
        throws_domain_error([&blank_description] {
            return api::json::reminder_from_json(blank_description);
        }),
        "uma descrição em branco deve ser rejeitada pelo domínio");

    return 0;
}
