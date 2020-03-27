open Belt;

Dotenv.config();

let env = Node.Process.process##env;

let postgresHost = Js.Dict.unsafeGet(env, "POSTGRES_HOST");
let postgresDatabase = Js.Dict.unsafeGet(env, "POSTGRES_NAME");
let postgresPort = Js.Dict.unsafeGet(env, "POSTGRES_PORT") |> int_of_string;
let postgresUser = Js.Dict.unsafeGet(env, "POSTGRES_USER");
let postgresPassword = Js.Dict.unsafeGet(env, "POSTGRES_PASSWORD");

let pgPool =
  BsPostgres.Pool.make(
    ~user=postgresUser,
    ~password=postgresPassword,
    ~host=postgresHost,
    ~database=postgresDatabase,
    ~port=postgresPort,
    (),
  );

let getClient = () => {
  let%Repromise.JsExn client = BsPostgres.Pool.Promise.connect(pgPool);
  Promise.resolved(client);
};

let releaseClient = client => {
  let%Repromise.JsExn _ = BsPostgres.Pool.Pool_Client.release(client);
  Promise.resolved();
};

type dbUserRow = {
  [@bs.as "access_token"]
  accessToken: string,
  [@bs.as "refresh_token"]
  refreshToken: string,
  [@bs.as "token_expire_time"]
  tokenExpireTime: Js.Date.t,
};

let getUser = (client, userId: string): Promise.t(option(User.t)) => {
  open BsPostgres.Client.Promise;
  let%Repromise.JsExn result =
    client
    |> query(
         "SELECT * FROM listeningbar.users WHERE id = $1",
         ~values=[|userId|],
       );
  Promise.resolved(
    Option.map(result##rows[0], (row) =>
      (
        {
          id: userId,
          accessToken: row.accessToken,
          refreshToken: row.refreshToken,
          tokenExpireTime: row.tokenExpireTime |> Js.Date.getTime,
        }: User.t
      )
    ),
  );
};

let updateUser = (client, user: User.t) => {
  open BsPostgres.Client.Promise;
  let%Repromise.Js result =
    client
    |> query'(
         BsPostgres.Query.make(
           ~text=
             {js|INSERT INTO listeningbar.users(id, access_token, refresh_token, token_expire_time)
          VALUES($1, $2, $3, $4)
          ON CONFLICT (id) DO UPDATE
          SET access_token=EXCLUDED.access_token, refresh_token=EXCLUDED.refresh_token, token_expire_time=EXCLUDED.token_expire_time|js},
           ~values=(
             user.id,
             user.accessToken,
             user.refreshToken,
             Js.Date.fromFloat(user.tokenExpireTime),
           ),
           (),
         ),
       );
  Promise.resolved();
};

type dbSessionRow = {
  id: string,
  [@bs.as "user_id"]
  userId: string,
};
let getSession = (client, sessionId: string) => {
  open BsPostgres.Client.Promise;
  let%Repromise.JsExn result =
    client
    |> query'(
         BsPostgres.Query.make(
           ~text={js|SELECT * FROM listeningbar.sessions WHERE id=$1|js},
           ~values=[|sessionId|],
           (),
         ),
       );
  Promise.resolved(
    Option.map(result##rows[0], (row) =>
      ({id: row.id, userId: row.userId}: User.session)
    ),
  );
};

let addSession = (client, session: User.session) => {
  open BsPostgres.Client.Promise;
  let%Repromise.Js result =
    client
    |> query'(
         BsPostgres.Query.make(
           ~text=
             {js|INSERT INTO listeningbar.sessions(id, user_id)
          VALUES($1, $2)
          ON CONFLICT (id) DO UPDATE
          SET user_id=EXCLUDED.user_id|js},
           ~values=(session.id, session.userId),
           (),
         ),
       );
  Promise.resolved();
};

let deleteSession = (client, sessionId: string) => {
  open BsPostgres.Client.Promise;
  let%Repromise.JsExn result =
    client
    |> query'(
         BsPostgres.Query.make(
           ~text={js|DELETE FROM listeningbar.sessions WHERE id=$1|js},
           ~values=[|sessionId|],
           (),
         ),
       );
  Promise.resolved();
};

type dbRoomRow = {
  id: string,
  record: Js.Json.t,
};
let getRoom = (client, roomId: string) => {
  open BsPostgres.Client.Promise;
  let%Repromise.JsExn result =
    client
    |> query'(
         BsPostgres.Query.make(
           ~text={js|SELECT * FROM listeningbar.rooms WHERE id=$1|js},
           ~values=[|roomId|],
           (),
         ),
       );
  Promise.resolved(
    Option.map(result##rows[0], (row: dbRoomRow) =>
      ({id: row.id, record: Obj.magic(row.record)}: Room.t)
    ),
  );
};

let updateRoom = (client, room: Room.t) => {
  open BsPostgres.Client.Promise;
  let%Repromise.Js result =
    client
    |> query'(
         BsPostgres.Query.make(
           ~text=
             {js|INSERT INTO listeningbar.rooms(id, record)
          VALUES($1, $2)
          ON CONFLICT (id) DO UPDATE
          SET record=EXCLUDED.record|js},
           ~values=(room.id, Js.Json.stringifyAny(room.record)),
           (),
         ),
       );
  Promise.resolved();
};