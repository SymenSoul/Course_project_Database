-- Database Schema for Dry Cleaner (Химчистка)
-- Designed for PostgreSQL

-- Drop views and tables if they exist (in reverse order of dependencies)
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
    final_price NUMERIC(10, 2) NOT NULL DEFAULT 0.00 CHECK (final_price >= 0.00),
    return_date TIMESTAMP
);

-- Trigger 2: Automatically calculate final price for each position
CREATE OR REPLACE FUNCTION set_order_position_price()
RETURNS TRIGGER AS $$
DECLARE
    v_base_cost NUMERIC(10, 2);
    v_discount NUMERIC(3, 2);
BEGIN
    SELECT base_cost INTO v_base_cost FROM service_type WHERE id = NEW.service_type_id;
    SELECT discount INTO v_discount FROM orders WHERE id = NEW.order_id;
    NEW.final_price := ROUND((v_base_cost * NEW.work_volume * NEW.complexity * NEW.urgency) * (1.0 - v_discount), 2);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_set_order_position_price
BEFORE INSERT OR UPDATE ON order_position
FOR EACH ROW
EXECUTE FUNCTION set_order_position_price();

-- 7. Database View for Order Positions
-- Dynamically calculates final price on query to prevent data duplication.

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

