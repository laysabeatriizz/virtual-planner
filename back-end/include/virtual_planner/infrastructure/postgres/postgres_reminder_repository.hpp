#pragma once

#include "virtual_planner/infrastructure/postgres/postgres_database.hpp"
#include "virtual_planner/persistence/reminder_repository.hpp"

#if defined(VIRTUAL_PLANNER_WITH_POSTGRES)

namespace virtual_planner::infrastructure::postgres {

class PostgresReminderRepository final
    : public persistence::ReminderRepository
{
public:
    explicit PostgresReminderRepository(
        PostgresDatabase& database);

    void save(const domain::Reminder& reminder) override;

    std::optional<domain::Reminder> find_by_id(
        std::uint64_t id) override;

    std::vector<domain::Reminder> find_all() override;

    void remove(std::uint64_t id) override;

private:
    PostgresDatabase& database_;
};

} // namespace virtual_planner::infrastructure::postgres

#endif
