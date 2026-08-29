#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include "virtual_planner/persistence/task_repository.hpp"

namespace virtual_planner::persistence {

// Repositorio de Task em memoria.
//
// TaskRepository nao expoe update, entao save faz upsert: substitui quem ja
// tem o mesmo id e insere caso contrario. Sem isso nao existe como alterar
// uma Task ja salva.
//
// Nao e thread-safe: o vector interno nao tem lock nenhum. O chamador deve
// serializar o acesso concorrente.
class InMemoryTaskRepository final : public TaskRepository
{
public:
    void save(const domain::Task& task,
              std::uint64_t user_id) override
    {
        for (auto& current : tasks_)
        {
            if (current.user_id == user_id && current.task.id() == task.id())
            {
                current.task = task;
                return;
            }
        }

        tasks_.push_back(StoredTask{user_id, task});
    }

    std::optional<domain::Task> find_by_id(std::uint64_t id,
                                           std::uint64_t user_id) override
    {
        for (const auto& task : tasks_)
        {
            if (task.user_id == user_id && task.task.id() == id)
            {
                return task.task;
            }
        }

        return std::nullopt;
    }

    std::vector<domain::Task> find_all(std::uint64_t user_id) override
    {
        std::vector<domain::Task> result;

        for (const auto& stored : tasks_)
        {
            if (stored.user_id == user_id)
            {
                result.push_back(stored.task);
            }
        }

        return result;
    }

    void remove(std::uint64_t id,
                std::uint64_t user_id) override
    {
        tasks_.erase(
            std::remove_if(
                tasks_.begin(),
                tasks_.end(),
                [id, user_id](const StoredTask& task)
                {
                    return task.user_id == user_id && task.task.id() == id;
                }),
            tasks_.end());
    }

private:
    struct StoredTask
    {
        std::uint64_t user_id;
        domain::Task task;
    };

    std::vector<StoredTask> tasks_;
};

} // namespace virtual_planner::persistence
