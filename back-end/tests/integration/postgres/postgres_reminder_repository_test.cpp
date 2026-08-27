#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include "virtual_planner/infrastructure/postgres/postgres_config.hpp"
#include "virtual_planner/infrastructure/postgres/postgres_database.hpp"
#include "virtual_planner/infrastructure/postgres/postgres_reminder_repository.hpp"

#include "support/expect.hpp"

using namespace virtual_planner;

namespace {

constexpr std::array<std::uint64_t, 6> kTestIds{
    900000000000040001ULL,
    900000000000040002ULL,
    900000000000040003ULL,
    900000000000040004ULL,
    900000000000040005ULL,
    900000000000040006ULL};

bool has_postgres_environment()
{
    return std::getenv("POSTGRES_DB") != nullptr &&
           std::getenv("POSTGRES_USER") != nullptr &&
           std::getenv("POSTGRES_PASSWORD") != nullptr;
}

void remove_test_reminders(
    infrastructure::postgres::PostgresReminderRepository& repository)
{
    for (const auto id : kTestIds)
    {
        repository.remove(id);
    }
}

void expect_reminder(
    const domain::Reminder& reminder,
    std::uint64_t id,
    const std::string& description,
    domain::Category category,
    const domain::Date& date,
    std::chrono::minutes start,
    std::chrono::minutes end,
    domain::ReminderType type,
    domain::ReminderRecurrence recurrence)
{
    VP_EXPECT(
        reminder.id() == id,
        "o id do lembrete deve ser preservado após a persistência");
    VP_EXPECT(
        reminder.description() == description,
        "a descrição do lembrete deve ser preservada após a persistência");
    VP_EXPECT(
        reminder.category() == category,
        "a categoria do lembrete deve ser preservada após a persistência");
    VP_EXPECT(
        reminder.date() == date,
        "a data do lembrete deve ser preservada após a persistência");
    VP_EXPECT(
        reminder.time_slot().start() == start,
        "o horário inicial do lembrete deve ser preservado em minutos");
    VP_EXPECT(
        reminder.time_slot().end() == end,
        "o horário final do lembrete deve ser preservado em minutos");
    VP_EXPECT(
        reminder.type() == type,
        "o tipo do lembrete deve ser preservado após a persistência");
    VP_EXPECT(
        reminder.recurrence() == recurrence,
        "a recorrência do lembrete deve ser preservada após a persistência");
}

} // namespace

int main()
{
    using infrastructure::postgres::PostgresConfig;
    using infrastructure::postgres::PostgresDatabase;
    using infrastructure::postgres::PostgresReminderRepository;

    if (!has_postgres_environment())
    {
        std::cout
            << "Teste do repositório PostgreSQL de lembretes ignorado: "
            << "POSTGRES_DB, POSTGRES_USER e POSTGRES_PASSWORD "
            << "são obrigatórias.\n";

        return 0;
    }

    try
    {
        // Preparação
        PostgresDatabase database(PostgresConfig::from_environment());
        database.initialize();
        database.connect();

        PostgresReminderRepository repository(database);
        remove_test_reminders(repository);

        const std::array<domain::Reminder, 6> reminders{
            domain::Reminder{
                kTestIds[0],
                "Reunião do projeto",
                domain::Category::Work,
                domain::Date{10, 9, 2026},
                domain::TimeSlot{
                    std::chrono::minutes{9 * 60 + 15},
                    std::chrono::minutes{10 * 60 + 45}},
                domain::ReminderType::Meeting,
                domain::ReminderRecurrence::Once},
            domain::Reminder{
                kTestIds[1],
                "Ligar para fornecedor",
                domain::Category::PersonalProjects,
                domain::Date{11, 9, 2026},
                domain::TimeSlot{
                    std::chrono::minutes{11 * 60},
                    std::chrono::minutes{11 * 60 + 30}},
                domain::ReminderType::PhoneCall,
                domain::ReminderRecurrence::Daily},
            domain::Reminder{
                kTestIds[2],
                "Comprar mantimentos",
                domain::Category::Leisure,
                domain::Date{12, 9, 2026},
                domain::TimeSlot{
                    std::chrono::minutes{12 * 60},
                    std::chrono::minutes{13 * 60}},
                domain::ReminderType::Shopping,
                domain::ReminderRecurrence::Weekly},
            domain::Reminder{
                kTestIds[3],
                "Revisar anotações de C++",
                domain::Category::Study,
                domain::Date{13, 9, 2026},
                domain::TimeSlot{
                    std::chrono::minutes{14 * 60},
                    std::chrono::minutes{15 * 60}},
                domain::ReminderType::Study,
                domain::ReminderRecurrence::Monthly},
            domain::Reminder{
                kTestIds[4],
                "Exercício matinal",
                domain::Category::Health,
                domain::Date{14, 9, 2026},
                domain::TimeSlot{
                    std::chrono::minutes{6 * 60},
                    std::chrono::minutes{7 * 60}},
                domain::ReminderType::Exercise,
                domain::ReminderRecurrence::Once},
            domain::Reminder{
                kTestIds[5],
                "Entregar atividade",
                domain::Category::College,
                domain::Date{15, 9, 2026},
                domain::TimeSlot{
                    std::chrono::minutes{20 * 60},
                    std::chrono::minutes{21 * 60}},
                domain::ReminderType::Assignment,
                domain::ReminderRecurrence::Daily}};

        // Execução
        for (const auto& reminder : reminders)
        {
            repository.save(reminder);
        }

        // Verificação: todos os valores de ReminderType e ReminderRecurrence
        // são preservados após a persistência.
        for (const auto& expected : reminders)
        {
            const auto stored = repository.find_by_id(expected.id());

            VP_EXPECT(
                stored.has_value(),
                "find_by_id() deve retornar cada lembrete salvo");

            expect_reminder(
                *stored,
                expected.id(),
                expected.description(),
                expected.category(),
                expected.date(),
                expected.time_slot().start(),
                expected.time_slot().end(),
                expected.type(),
                expected.recurrence());
        }

        VP_EXPECT(
            !repository.find_by_id(900000000000049999ULL).has_value(),
            "find_by_id() deve retornar nullopt para um id inexistente");

        const auto all_reminders = repository.find_all();

        for (const auto id : kTestIds)
        {
            const auto found = std::any_of(
                all_reminders.begin(),
                all_reminders.end(),
                [id](const domain::Reminder& reminder)
                {
                    return reminder.id() == id;
                });

            VP_EXPECT(
                found,
                "find_all() deve incluir todos os lembretes salvos pelo teste");
        }

        // Execução: save() com o mesmo id deve atualizar o registro existente
        // em vez de inserir outro.
        const domain::Reminder updated{
            kTestIds[0],
            "Lembrete atualizado",
            domain::Category::College,
            domain::Date{29, 2, 2024},
            domain::TimeSlot{
                std::chrono::minutes{1439},
                std::chrono::minutes{1440}},
            domain::ReminderType::Assignment,
            domain::ReminderRecurrence::Monthly};

        repository.save(updated);

        const auto reloaded = repository.find_by_id(kTestIds[0]);

        VP_EXPECT(
            reloaded.has_value(),
            "o lembrete atualizado deve continuar disponível para consulta");

        expect_reminder(
            *reloaded,
            updated.id(),
            updated.description(),
            updated.category(),
            updated.date(),
            updated.time_slot().start(),
            updated.time_slot().end(),
            updated.type(),
            updated.recurrence());

        const auto after_upsert = repository.find_all();
        const auto matching_id_count = std::count_if(
            after_upsert.begin(),
            after_upsert.end(),
            [](const domain::Reminder& reminder)
            {
                return reminder.id() == kTestIds[0];
            });

        VP_EXPECT(
            matching_id_count == 1,
            "save() com um id existente deve manter apenas um registro");

        // Execução e verificação: remover um id existente ou inexistente deve
        // ser seguro.
        repository.remove(kTestIds[0]);
        VP_EXPECT(
            !repository.find_by_id(kTestIds[0]).has_value(),
            "remove() deve excluir um lembrete existente");

        repository.remove(kTestIds[0]);
        VP_EXPECT(
            !repository.find_by_id(kTestIds[0]).has_value(),
            "remove() de um id inexistente não deve causar alteração");

        remove_test_reminders(repository);
        database.shutdown();

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
