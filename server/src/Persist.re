module LowDb = {
  type t;
  type adapter;

  [@bs.module] external makeDb: adapter => t = "lowdb";
  [@bs.module] [@bs.new]
  external makeAdapter: string => adapter = "lowdb/adapters/FileSync";

  [@bs.send] external get: (t, string) => t = "get";
  [@bs.send] external find: (t, 'query) => t = "find";

  type mutation;
  [@bs.send] external defaults: (t, 'data) => mutation = "defaults";
  [@bs.send] external push: (t, 'data) => mutation = "push";
  [@bs.send] external set: (t, string, 'data) => mutation = "set";
  [@bs.send] external write: mutation => unit = "write";

  [@bs.send] external value: t => option('data) = "value";
};

let adapter = LowDb.makeAdapter("db.json");
let db = LowDb.makeDb(adapter);

LowDb.defaults(
  db,
  {
    "users": Js.Dict.empty(),
    "sessions": Js.Dict.empty(),
    "rooms": Js.Dict.empty(),
  },
)
->LowDb.write;

let updateUser = (user: User.t) => {
  LowDb.(
    db
    ->get("users")
    ->set(
        user.id,
        {
          "accessToken": user.accessToken,
          "refreshToken": user.refreshToken,
          "tokenExpireTime": user.tokenExpireTime,
        },
      )
    ->write
  );
};

let addSession = (session: User.session) => {
  LowDb.(
    db->get("sessions")->set(session.id, {"userId": session.userId})->write
  );
};

let getSession = (sessionId: string): option(User.session) => {
  LowDb.(db->get("sessions")->get(sessionId)->value);
};

let getUser = (userId: string): option(User.t) => {
  LowDb.(db->get("users")->get(userId)->value);
};

let updateRoom = (room: Room.t) => {
  LowDb.(
    db->get("rooms")->set(room.id, {"trackState": room.trackState})->write
  );
};

let getRoom = (roomId: string): option(Room.t) => {
  LowDb.(db->get("rooms")->get(roomId)->value);
};