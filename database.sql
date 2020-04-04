--
-- PostgreSQL database dump
--

-- Dumped from database version 9.5.19
-- Dumped by pg_dump version 11.5

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: listeningbar; Type: SCHEMA; Schema: -;
--

CREATE SCHEMA listeningbar;


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: rooms; Type: TABLE; Schema: listeningbar;
--

CREATE TABLE listeningbar.rooms (
    id text NOT NULL,
    record jsonb
);


--
-- Name: sessions; Type: TABLE; Schema: listeningbar;
--

CREATE TABLE listeningbar.sessions (
    id text NOT NULL,
    user_id text,
    created timestamp with time zone DEFAULT now()
);


--
-- Name: users; Type: TABLE; Schema: listeningbar;
--

CREATE TABLE listeningbar.users (
    id text NOT NULL,
    access_token text,
    refresh_token text,
    token_expire_time timestamp with time zone,
    created timestamp with time zone DEFAULT now(),
    anonymous boolean
);


--
-- Name: rooms rooms_pkey; Type: CONSTRAINT; Schema: listeningbar;
--

ALTER TABLE ONLY listeningbar.rooms
    ADD CONSTRAINT rooms_pkey PRIMARY KEY (id);


--
-- Name: sessions sessions_pkey; Type: CONSTRAINT; Schema: listeningbar;
--

ALTER TABLE ONLY listeningbar.sessions
    ADD CONSTRAINT sessions_pkey PRIMARY KEY (id);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: listeningbar;
--

ALTER TABLE ONLY listeningbar.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- Name: sessions sessions_user_id_fkey; Type: FK CONSTRAINT; Schema: listeningbar;
--

ALTER TABLE ONLY listeningbar.sessions
    ADD CONSTRAINT sessions_user_id_fkey FOREIGN KEY (user_id) REFERENCES listeningbar.users(id);


--
-- PostgreSQL database dump complete
--

