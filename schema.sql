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

