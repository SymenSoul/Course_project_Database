-- Database Schema for Dry Cleaner (Химчистка)
-- Designed for PostgreSQL

-- Drop views and tables if they exist (in reverse order of dependencies)
DROP VIEW IF EXISTS order_position_view CASCADE;
DROP TABLE IF EXISTS order_position CASCADE;
DROP TABLE IF EXISTS orders CASCADE;
DROP TABLE IF EXISTS client CASCADE;
DROP TABLE IF EXISTS users CASCADE;
DROP TABLE IF EXISTS service_type CASCADE;
DROP TABLE IF EXISTS branch CASCADE;

-- 1. Branch Table (Филиал)
CREATE TABLE branch (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    address VARCHAR(255) NOT NULL
);

-- 2. Service Type Table (Вид услуги)
CREATE TABLE service_type (
    id SERIAL PRIMARY KEY,
    name VARCHAR(150) NOT NULL,
    type VARCHAR(100) NOT NULL,
    base_cost NUMERIC(10, 2) NOT NULL CHECK (base_cost >= 0)
);

-- 3. Users Table (Пользователь)
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    role VARCHAR(20) NOT NULL CHECK (role IN ('Admin', 'Employee')),
    full_name VARCHAR(150) NOT NULL
);

-- 4. Client Table (Клиент)
CREATE TABLE client (
    id SERIAL PRIMARY KEY,
    last_name VARCHAR(50) NOT NULL,
    first_name VARCHAR(50) NOT NULL,
    middle_name VARCHAR(50),
    phone VARCHAR(20) NOT NULL
);

-- 5. Orders Table (Заказ)
-- Stores the discount at the moment of order placement to preserve historical integrity.
CREATE TABLE orders (
    id SERIAL PRIMARY KEY,
    branch_id INTEGER NOT NULL REFERENCES branch(id) ON DELETE RESTRICT,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE RESTRICT,
    client_id INTEGER NOT NULL REFERENCES client(id) ON DELETE RESTRICT,
    intake_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    discount NUMERIC(3, 2) NOT NULL DEFAULT 0.00 CHECK (discount >= 0.00 AND discount <= 1.00)
);

-- 6. Order Position Table (Позиция заказа)
-- Fully normalized: does NOT contain derived/calculated final_price column.
CREATE TABLE order_position (
    id SERIAL PRIMARY KEY,
    order_id INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    service_type_id INTEGER NOT NULL REFERENCES service_type(id) ON DELETE RESTRICT,
    item_name VARCHAR(150) NOT NULL,
    complexity NUMERIC(3, 2) NOT NULL DEFAULT 1.00 CHECK (complexity >= 1.0),
    urgency NUMERIC(3, 2) NOT NULL DEFAULT 1.00 CHECK (urgency >= 1.0),
    work_volume NUMERIC(10, 2) NOT NULL DEFAULT 1.00 CHECK (work_volume > 0),
    return_date TIMESTAMP
);

-- 7. Database View for Order Positions
-- Dynamically calculates final price on query to prevent data duplication.
CREATE VIEW order_position_view AS
SELECT op.id,
       op.order_id,
       op.service_type_id,
       op.item_name,
       op.complexity,
       op.urgency,
       op.work_volume,
       op.return_date,
       s.name AS service_name,
       s.base_cost,
       o.discount,
       ROUND((s.base_cost * op.work_volume * op.complexity * op.urgency) * (1.0 - o.discount), 2) AS final_price
FROM order_position op
JOIN service_type s ON op.service_type_id = s.id
JOIN orders o ON op.order_id = o.id;

-- Indices for performance
CREATE INDEX idx_orders_client ON orders(client_id);
CREATE INDEX idx_order_position_order ON order_position(order_id);

-- =========================================================================
-- Triggers for Data Integrity & Business Logic Automation
-- =========================================================================

-- Trigger: Automatically calculate order discount at the moment of order placement
CREATE OR REPLACE FUNCTION set_order_discount()
RETURNS TRIGGER AS $$
DECLARE
    order_count INT;
BEGIN
    SELECT COUNT(*) INTO order_count FROM orders WHERE client_id = NEW.client_id;
    IF order_count >= 2 THEN
        NEW.discount := 0.03;
    ELSE
        NEW.discount := 0.00;
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_set_order_discount
BEFORE INSERT ON orders
FOR EACH ROW
EXECUTE FUNCTION set_order_discount();

-- =========================================================================
-- Initial Seed Data
-- =========================================================================

-- Branches
INSERT INTO branch (name, address) VALUES
('Центральный филиал', 'ул. Ленина, д. 10'),
('Северный филиал', 'пр. Мира, д. 45'),
('Западный филиал', 'ул. Гагарина, д. 12');

-- Service Types
INSERT INTO service_type (name, type, base_cost) VALUES
('Сухая химчистка куртки', 'Химчистка', 1200.00),
('Чистка шерстяного пальто', 'Химчистка', 1800.00),
('Аквачистка пуховика', 'Аквачистка', 1500.00),
('Выведение пятен с платья', 'Пятновыведение', 800.00),
('Чистка ковра', 'Специальная чистка', 350.00),
('Глажка костюма', 'Утюжка', 500.00);

-- Users (Default passwords: Admin -> StrongPass!123, Employee -> Employee!123)
INSERT INTO users (username, password_hash, role, full_name) VALUES
('admin', '924c8f81aa37dea3e8206314ccae02068f5a905a08fc8cab884302d30461aaa3', 'Admin', 'Иванов Иван Иванович'),
('employee', '57d1aadf4bf62b3b34b7d4584315d86e630baf1297eb874a0ab8d89c4edb5a09', 'Employee', 'Петров Петр Петрович');

-- Test Clients
INSERT INTO client (last_name, first_name, middle_name, phone) VALUES
('Сидоров', 'Алексей', 'Сергеевич', '+7 (999) 111-22-33'),
('Кузнецова', 'Мария', 'Игоревна', '+7 (999) 444-55-66');
