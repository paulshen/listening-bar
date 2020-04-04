# Listening Bar

## Installation

```sh
yarn
(cd server && yarn)
cp .env.sample .env
```

Replace contents of `.env` with your values.

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
