-- A recorrência é armazenada como uma regra ancorada em reminder_date.
-- As ocorrências são expandidas pela camada de aplicação e não são materializadas no PostgreSQL.
CREATE TABLE reminders
(
    id BIGINT PRIMARY KEY,

    user_id BIGINT NOT NULL REFERENCES users(id),

    description TEXT NOT NULL,

    category VARCHAR(50) NOT NULL,

    reminder_date DATE NOT NULL,

    start_minutes INTEGER NOT NULL,

    end_minutes INTEGER NOT NULL,

    type VARCHAR(50) NOT NULL,

    recurrence VARCHAR(50) NOT NULL,

    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT reminders_category_check
    CHECK (category IN (
        'College',
        'Work',
        'Health',
        'Leisure',
        'PersonalProjects',
        'Study'
    )),

    CONSTRAINT reminders_type_check
    CHECK (type IN (
        'Meeting',
        'PhoneCall',
        'Shopping',
        'Study',
        'Exercise',
        'Assignment'
    )),

    CONSTRAINT reminders_recurrence_check
    CHECK (recurrence IN (
        'Once',
        'Daily',
        'Weekly',
        'Monthly'
    ))
);
