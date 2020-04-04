# Listening Bar

https://listening.bar

A ReasonML/BuckleScript webapp for listening to Spotify albums together.

## Installation

```sh
yarn
(cd server && yarn)
cp .env.sample .env
```

Replace contents of `.env` with your values.

#### Database

Listening Bar uses Postgres. Use `database.sql` to create the necessary tables.

## Run

#### Client

```sh
# Build BuckleScript
yarn build
# Serve dev client on localhost:8000
yarn client
```

#### Server

```sh
# Build BuckleScript
(cd server && yarn build)
# Run server on localhost:3030
yarn server
```

Replace `yarn build` with `yarn start` to run BuckleScript in watch mode.
