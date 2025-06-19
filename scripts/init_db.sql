CREATE DATABASE polling_app;

\c polling_app

CREATE TABLE polls (
    id SERIAL PRIMARY KEY,
    question TEXT NOT NULL,
    created_at TIMESTAMP NOT NULL
);

CREATE TABLE poll_options (
    id SERIAL PRIMARY KEY,
    poll_id INTEGER REFERENCES polls(id),
    option_text TEXT NOT NULL
);

CREATE TABLE votes (
    id SERIAL PRIMARY KEY,
    poll_id INTEGER REFERENCES polls(id),
    option_id INTEGER REFERENCES poll_options(id),
    user_ip TEXT NOT NULL,
    created_at TIMESTAMP NOT NULL
);
