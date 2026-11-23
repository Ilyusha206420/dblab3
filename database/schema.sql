DROP TABLE IF EXISTS "component_costs" CASCADE;
DROP TABLE IF EXISTS "services" CASCADE;
DROP TABLE IF EXISTS "guitars" CASCADE;
DROP TABLE IF EXISTS "components" CASCADE;
DROP TABLE IF EXISTS "masters" CASCADE;
DROP TABLE IF EXISTS "clients" CASCADE;
DROP TABLE IF EXISTS "app_users" CASCADE;

CREATE TABLE "app_users" (
    "id" SERIAL PRIMARY KEY,
    "username" varchar(50) UNIQUE NOT NULL,
    "password" varchar(100) NOT NULL,
    "role" varchar(20) NOT NULL CHECK (role IN ('admin', 'master', 'manager'))
);

CREATE TABLE "clients" (
    "id" SERIAL PRIMARY KEY,
    "fullname" varchar(150) NOT NULL,
    "mobile_number" varchar(30),
    "email" varchar(100)
);

CREATE TABLE "guitars" (
    "id" SERIAL PRIMARY KEY,
    "client_id" integer NOT NULL REFERENCES "clients" ("id") ON DELETE CASCADE,
    "type" varchar(50) NOT NULL,
    "model" varchar(100) NOT NULL,
    "serial_number" varchar(100) UNIQUE
);

CREATE TABLE "masters" (
    "id" SERIAL PRIMARY KEY,
    "fullname" varchar(150) NOT NULL,
    "specialization" varchar(100)
);

CREATE TABLE "services" (
    "id" SERIAL PRIMARY KEY,
    "master_id" integer REFERENCES "masters" ("id") ON DELETE SET NULL,
    "name" varchar(150) NOT NULL,
    "cost" numeric(10, 2) NOT NULL CHECK (cost >= 0)
);

CREATE TABLE "components" (
    "id" SERIAL PRIMARY KEY,
    "name" varchar(150) NOT NULL,
    "cost" numeric(10, 2) NOT NULL CHECK (cost >= 0)
);

CREATE TABLE "component_costs" (
    "components_id" integer REFERENCES "components" ("id") ON DELETE CASCADE,
    "clients_id" integer REFERENCES "clients" ("id") ON DELETE CASCADE,
    "services_id" integer REFERENCES "services" ("id") ON DELETE CASCADE,
    PRIMARY KEY ("components_id", "clients_id", "services_id")
);

INSERT INTO "app_users" ("username", "password", "role") VALUES
('admin', 'admin123', 'admin'),
('master', 'master123', 'master'),
('manager', 'manager123', 'manager');

INSERT INTO "clients" ("fullname", "mobile_number", "email") VALUES
('Иванов Алексей Сергеевич', '+7-999-111-22-33', 'ivanov@mail.ru'),
('Смирнова Елена Дмитриевна', '+7-921-444-55-66', 'elena.s@yandex.ru'),
('Тестовый клиент для проверки переноса текста', '+7-000-000-00-00', 'wrap-test@example.invalid');

INSERT INTO "guitars" ("client_id", "type", "model", "serial_number") VALUES
(1, 'Электрогитара', 'Fender Stratocaster', 'US2100458'),
(1, 'Электрогитара', 'Gibson Les Paul Standard', 'LP994812'),
(2, 'Акустическая', 'Yamaha FG800', 'YM883210');

INSERT INTO "masters" ("fullname", "specialization") VALUES
('Петров Дмитрий (Лад-мастер)', 'Шлифовка и замена ладов, отстройка'),
('Соколов Игорь (Электронщик)', 'Экранировка, пайка темброблоков, звукосниматели');

INSERT INTO "services" ("master_id", "name", "cost") VALUES
(1, 'Замена ладов (нержавейка)', 8500.00),
(1, 'Полная отстройка мензуры и анкера', 2500.00),
(2, 'Замена звукоснимателей и экранировка', 4000.00),
(1, 'Полная настройка электрогитары с заменой ладов, полировкой грифа и регулировкой анкера', 7500.00);

INSERT INTO "components" ("name", "cost") VALUES
('Лады Sintoms 2.5мм (нейзильбер)', 1200.00),
('Потенциометр CTS 500k', 950.00),
('Хамбакер Seymour Duncan JB', 11500.00),
('Комплект ладов из нержавеющей стали с увеличенным профилем для точной настройки электрогитары', 3200.00);

INSERT INTO "component_costs" ("components_id", "clients_id", "services_id") VALUES
(1, 1, 1),
(3, 1, 3),
(2, 2, 3),
(4, 3, 4);
