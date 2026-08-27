#include "virtual_planner/infrastructure/postgres/postgres_reminder_repository.hpp"

#if defined(VIRTUAL_PLANNER_WITH_POSTGRES)

#include <chrono>
#include <cstdint>
#include <pqxx/pqxx>

namespace virtual_planner::infrastructure::postgres {

namespace {

constexpr std::uint64_t kSingleTenantUserId{1};

domain::Reminder reminder_from_row(const pqxx::row_ref& row)
{
    return domain::Reminder{
        row["id"].as<std::uint64_t>(),
        row["description"].as<std::string>(),
        domain::category_from_string(
            row["category"].as<std::string>()),
        domain::Date{
            row["date_day"].as<std::uint32_t>(),
            row["date_month"].as<std::uint32_t>(),
            row["date_year"].as<std::uint32_t>()},
        domain::TimeSlot{
            std::chrono::minutes{
                row["start_minutes"].as<std::int64_t>()},
            std::chrono::minutes{
                row["end_minutes"].as<std::int64_t>()}},
        domain::reminder_type_from_string(
            row["type"].as<std::string>()),
        domain::reminder_recurrence_from_string(
            row["recurrence"].as<std::string>())};
}

}

PostgresReminderRepository::PostgresReminderRepository(
    PostgresDatabase& database)
    : database_(database)
{
}

void PostgresReminderRepository::save(
    const domain::Reminder& reminder)
{
    pqxx::work transaction(database_.connection());

    transaction.exec(
        R"(
            INSERT INTO reminders
            (
                id,
                user_id,
                description,
                category,
                reminder_date,
                start_minutes,
                end_minutes,
                type,
                recurrence
            )
            VALUES ($1, $2, $3, $4, make_date($5, $6, $7), $8, $9, $10, $11)
            ON CONFLICT (id) DO UPDATE
            SET
                description = EXCLUDED.description,
                category = EXCLUDED.category,
                reminder_date = EXCLUDED.reminder_date,
                start_minutes = EXCLUDED.start_minutes,
                end_minutes = EXCLUDED.end_minutes,
                type = EXCLUDED.type,
                recurrence = EXCLUDED.recurrence,
                updated_at = CURRENT_TIMESTAMP
            WHERE reminders.user_id = EXCLUDED.user_id
        )",
        pqxx::params{
            transaction,
            reminder.id(),
            kSingleTenantUserId,
            reminder.description(),
            domain::to_string(reminder.category()),
            reminder.date().year(),
            reminder.date().month(),
            reminder.date().day(),
            reminder.time_slot().start().count(),
            reminder.time_slot().end().count(),
            domain::to_string(reminder.type()),
            domain::to_string(reminder.recurrence())}).no_rows();

    transaction.commit();
}

std::optional<domain::Reminder>
PostgresReminderRepository::find_by_id(std::uint64_t id)
{
    pqxx::read_transaction transaction(database_.connection());

    const auto result = transaction.exec(
        R"(
            SELECT
                id,
                description,
                category,
                EXTRACT(DAY FROM reminder_date)::INTEGER AS date_day,
                EXTRACT(MONTH FROM reminder_date)::INTEGER AS date_month,
                EXTRACT(YEAR FROM reminder_date)::INTEGER AS date_year,
                start_minutes,
                end_minutes,
                type,
                recurrence
            FROM reminders
            WHERE id = $1 AND user_id = $2
        )",
        pqxx::params{transaction, id, kSingleTenantUserId});

    if (result.empty())
    {
        return std::nullopt;
    }

    return reminder_from_row(result.front());
}

std::vector<domain::Reminder>
PostgresReminderRepository::find_all()
{
    pqxx::read_transaction transaction(database_.connection());

    const auto result = transaction.exec(
        R"(
            SELECT
                id,
                description,
                category,
                EXTRACT(DAY FROM reminder_date)::INTEGER AS date_day,
                EXTRACT(MONTH FROM reminder_date)::INTEGER AS date_month,
                EXTRACT(YEAR FROM reminder_date)::INTEGER AS date_year,
                start_minutes,
                end_minutes,
                type,
                recurrence
            FROM reminders
            WHERE user_id = $1
            ORDER BY id
        )",
        pqxx::params{transaction, kSingleTenantUserId});

    std::vector<domain::Reminder> reminders;
    reminders.reserve(result.size());

    for (const auto& row : result)
    {
        reminders.push_back(reminder_from_row(row));
    }

    return reminders;
}

void PostgresReminderRepository::remove(std::uint64_t id)
{
    pqxx::work transaction(database_.connection());

    transaction.exec(
        "DELETE FROM reminders WHERE id = $1 AND user_id = $2",
        pqxx::params{transaction, id, kSingleTenantUserId}).no_rows();

    transaction.commit();
}

} // namespace virtual_planner::infrastructure::postgres

#endif
