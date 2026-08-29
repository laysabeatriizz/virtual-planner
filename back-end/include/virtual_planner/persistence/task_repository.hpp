#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "virtual_planner/domain/entities/task.hpp"

namespace virtual_planner::persistence {

class TaskRepository
{
public:
    virtual ~TaskRepository() = default;

    virtual void save(const domain::Task& task,
                      std::uint64_t user_id) = 0;

    virtual std::optional<domain::Task> find_by_id(
        std::uint64_t id,
        std::uint64_t user_id) = 0;

    virtual std::vector<domain::Task> find_all(
        std::uint64_t user_id) = 0;

    virtual void remove(std::uint64_t id,
                        std::uint64_t user_id) = 0;
};

} // namespace virtual_planner::persistence
