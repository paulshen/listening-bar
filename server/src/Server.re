open Belt;
open Express;

Dotenv.config();

let env = Node.Process.process##env;
let adminUserId = Js.Dict.unsafeGet(env, "ADMIN_USER_ID");

[@bs.module]
external bodyParser: {. [@bs.meth] "json": unit => Middleware.t} =
  "body-parser";
[@bs.module] external cors: unit => Middleware.t = "cors";
let app = express();

App.disable(app, ~name="x-powered-by");
App.use(app, bodyParser##json());
App.use(app, cors());

module SocketServer = BsSocket.Server.Make(SocketMessage);

let promiseMiddleware = middleware => {
  PromiseMiddleware.from((next, req, res) =>
    Promise.Js.toBsPromise(middleware(next, req, res))
  );
};
let setProperty = (req, property, value, res) => {
  let reqData = Request.asJsonObject(req);
  Js.Dict.set(reqData, property, value);
  res;
};
let getProperty = (req, property) => {
  let reqData = Request.asJsonObject(req);
  Js.Dict.get(reqData, property);
};

let apiRouter = router();

Router.get(apiRouter, ~path="/hello") @@
promiseMiddleware((next, req, res) =>
  res |> Response.sendString("Hello World!") |> Promise.Js.resolved
);

Router.post(apiRouter, ~path="/login") @@
promiseMiddleware((next, req, res) => {
  let bodyJson = Request.bodyJSON(req) |> Option.getExn;
  let code =
    bodyJson
    ->Js.Json.decodeObject
    ->Option.getExn
    ->Js.Dict.unsafeGet("code")
    ->Js.Json.decodeString
    ->Option.getExn;
  let%Repromise (sessionId, accessToken, userId, anonymous) =
    Spotify.getToken(code);
  let responseJson =
    Js.Json.object_(
      Js.Dict.fromArray([|
        ("sessionId", Js.Json.string(sessionId)),
        ("accessToken", Js.Json.string(accessToken)),
        ("userId", Js.Json.string(userId)),
        ("anonymous", Js.Json.boolean(anonymous)),
      |]),
    );
  res |> Response.sendJson(responseJson) |> Promise.resolved;
});

Router.post(apiRouter, ~path="/logout") @@
promiseMiddleware((next, req, res) => {
  let sessionId = Request.get("Authorization", req);
  let%Repromise _ =
    switch (sessionId) {
    | Some(sessionId) =>
      let%Repromise client = Database.getClient();
      let%Repromise _ = Database.deleteSession(client, sessionId);
      Database.releaseClient(client);
    | None => Promise.resolved()
    };
  res |> Response.sendStatus(Ok) |> Promise.resolved;
});

let getAccessTokenForUserOption = user => {
  let%Repromise accessToken =
    switch ((user: option(User.t))) {
    | Some(user) =>
      if (user.tokenExpireTime < Js.Date.now()) {
        let%Repromise (accessToken, tokenExpireTime) =
          Spotify.refreshToken(user.refreshToken);
        let%Repromise client = Database.getClient();
        let%Repromise _ =
          Database.updateUser(
            client,
            {...user, id: user.id, accessToken, tokenExpireTime},
          );
        Database.releaseClient(client) |> ignore;
        Promise.resolved(Some(accessToken));
      } else {
        Promise.resolved(Some(user.accessToken));
      }
    | None => Promise.resolved(None)
    };
  Promise.resolved(accessToken);
};

Router.getWithMany(apiRouter, ~path="/user") @@
[|
  promiseMiddleware((next, req, res) => {
    let%Repromise client = Database.getClient();
    let%Repromise authorized =
      switch (Request.get("Authorization", req)) {
      | Some(sessionId) =>
        let%Repromise session = Database.getSession(client, sessionId);
        switch (session) {
        | Some(session) =>
          setProperty(req, "session", Obj.magic(session), res) |> ignore;
          setProperty(req, "client", Obj.magic(client), res) |> ignore;
          Promise.resolved(true);
        | None => Promise.resolved(false)
        };
      | None => Promise.resolved(false)
      };

    if (authorized) {
      next(Next.middleware, res) |> Promise.resolved;
    } else {
      Database.releaseClient(client) |> ignore;
      Response.sendStatus(Response.StatusCode.Unauthorized, res)
      |> Promise.resolved;
    };
  }),
  promiseMiddleware((next, req, res) => {
    let client = Obj.magic(Option.getExn(getProperty(req, "client")));
    let userId =
      Obj.magic(Option.getExn(getProperty(req, "session")))##userId;
    let%Repromise user = Database.getUser(client, userId);
    Database.releaseClient(client) |> ignore;
    let%Repromise accessToken = getAccessTokenForUserOption(user);
    let responseJson =
      Js.Json.object_(
        Js.Dict.fromArray([|
          (
            "userId",
            switch (userId) {
            | Some(userId) => Js.Json.string(userId)
            | None => Js.Json.null
            },
          ),
          (
            "accessToken",
            switch (accessToken) {
            | Some(accessToken) => Js.Json.string(accessToken)
            | None => Js.Json.null
            },
          ),
          (
            "anonymous",
            Js.Json.boolean(
              switch (user) {
              | Some(user) => user.anonymous
              | None => false
              },
            ),
          ),
        |]),
      );
    res |> Response.sendJson(responseJson) |> Promise.resolved;
  }),
|];

App.useRouterOnPath(app, ~path="/", apiRouter);

let onListen = e =>
  switch (e) {
  | exception (Js.Exn.Error(e)) =>
    Js.log(e);
    Node.Process.exit(1);
  | _ => Js.log @@ "Listening at http://127.0.0.1:3030"
  };

let server = App.listen(app, ~port=3030, ~onListen, ());

let roomConnections: Js.Dict.t(array(SocketMessage.connection)) =
  Js.Dict.empty();

let io = SocketServer.createWithHttp(server);
Router.get(apiRouter, ~path="/rooms/:roomId") @@
Middleware.from((next, req, res) => {
  let roomId =
    Request.params(req)
    ->Js.Dict.unsafeGet("roomId")
    ->Js.Json.decodeString
    ->Option.getExn
    ->Utils.sanitizeRoomId;
  let roomId = Js.String.toLowerCase(roomId);
  let connections = roomConnections->Js.Dict.get(roomId);
  switch (connections) {
  | Some(connections) =>
    res
    |> Response.sendJson(
         Js.Json.array(
           connections
           |> Js.Array.map((connection: SocketMessage.connection) =>
                Js.Json.stringArray([|connection.id, connection.userId|])
              ),
         ),
       )
  | None => res |> Response.sendStatus(Ok)
  };
});

[@bs.send]
external inRoom:
  (BsSocket.Server.serverT, BsSocket.Server.room) => BsSocket.Server.socketT =
  "in";

let removeRoomConnection = (socket, roomId, socketId) => {
  SocketServer.(
    switch (Js.Dict.get(roomConnections, roomId)) {
    | Some(connections) =>
      socket
      ->SocketServer.Socket.to_(roomId)
      ->Socket.emit(LostConnection(roomId, socketId));
      let updatedConnections =
        connections
        |> Js.Array.filter((connection: SocketMessage.connection) =>
             connection.id != socketId
           );
      roomConnections->Js.Dict.set(roomId, updatedConnections);
    | None => ()
    }
  );
};

SocketServer.onConnect(
  io,
  socket => {
    open SocketServer;
    let socketId = Socket.getId(socket);
    let roomIdRef = ref(None);
    Socket.on(
      socket,
      message => {
        Js.log2(Js.Date.now(), message);
        switch (message) {
        | JoinRoom(roomId, sessionId) =>
          {
            switch (roomIdRef^) {
            | Some(roomId) => removeRoomConnection(socket, roomId, socketId)
            | None => ()
            };

            let roomId = Js.String.toLowerCase(Utils.sanitizeRoomId(roomId));
            socket->Socket.join(roomId) |> ignore;
            let%Repromise client = Database.getClient();
            let%Repromise (
              session: option(User.session),
              room: option(Room.t),
            ) =
              Promise.all2(
                if (sessionId == "") {
                  Promise.resolved(None);
                } else {
                  let%Repromise session =
                    Database.getSession(client, sessionId);
                  Promise.resolved(session);
                },
                Database.getRoom(client, roomId),
              );
            let%Repromise user =
              switch (session) {
              | Some(session) => Database.getUser(client, session.userId)
              | None => Promise.resolved(None)
              };
            Database.releaseClient(client) |> ignore;
            let connectionUserId =
              switch (
                switch (user) {
                | Some(user) => user.anonymous
                | None => false
                },
                session,
              ) {
              | (_, None) => ""
              | (true, _) => "anonymous"
              | (false, Some(session)) => session.userId
              };
            let connection: SocketMessage.connection = {
              id: socketId,
              userId: connectionUserId,
            };
            socket
            ->Socket.to_(roomId)
            ->Socket.emit(
                NewConnection(
                  roomId,
                  SocketMessage.serializeConnection(connection),
                ),
              );
            let room =
              Option.getWithDefault(room, {id: roomId, record: None});
            let connections =
              Option.getWithDefault(
                Js.Dict.get(roomConnections, roomId),
                [||],
              );
            let hasRecordEnded =
              !(Constants.foreverRoomIds |> Js.Array.includes(roomId))
              && (
                switch (room.record) {
                | Some((_userId, _albumId, serializedTracks, startTimestamp)) =>
                  let currentTrack =
                    SocketMessage.getCurrentTrack(
                      serializedTracks
                      |> Js.Array.map(
                           (
                             (_, _, _, _, _, _, duration): SocketMessage.serializedRoomTrack,
                           ) =>
                           duration
                         ),
                      startTimestamp,
                      false,
                    );
                  Belt.Option.isNone(currentTrack);
                | None => false
                }
              );
            let updatedRoom =
              if (hasRecordEnded) {
                {...room, record: None};
              } else {
                room;
              };
            let updatedConnections =
              if (connections
                  |> Js.Array.findIndex(
                       (connection: SocketMessage.connection) =>
                       connection.id == socketId
                     )
                  == (-1)) {
                connections |> Js.Array.concat([|connection|]);
              } else {
                connections;
              };
            roomConnections->Js.Dict.set(roomId, updatedConnections);
            socket->Socket.emit(
              Connected(
                roomId,
                updatedConnections
                |> Js.Array.map(SocketMessage.serializeConnection),
                switch (updatedRoom.record) {
                | None => ""
                | Some((userId, _, _, _)) => userId
                },
                switch (updatedRoom.record) {
                | None => ""
                | Some((_, albumId, _, _)) => albumId
                },
                switch (updatedRoom.record) {
                | None => [||]
                | Some((_, _, serializedTracks, _)) => serializedTracks
                },
                switch (updatedRoom.record) {
                | None => 0.
                | Some((_, _, _, startTimestamp)) => startTimestamp
                },
              ),
            );
            roomIdRef := Some(roomId);
            Promise.resolved();
          }
          |> ignore
        | PublishTrackState(sessionId, roomId, trackState) =>
          let roomId = Js.String.toLowerCase(Utils.sanitizeRoomId(roomId));
          (
            switch (roomIdRef^) {
            | Some(storedRoomId) =>
              let (trackId, contextType, contextId, startTimestamp) = trackState;
              let albumId = contextId;
              let%Repromise client = Database.getClient();
              let%Repromise session = Database.getSession(client, sessionId);
              let userId = Option.map(session, session => session.userId);
              let%Repromise user =
                switch (userId) {
                | Some(userId) => Database.getUser(client, userId)
                | None => Promise.resolved(None)
                };
              Database.releaseClient(client) |> ignore;
              let%Repromise accessToken = getAccessTokenForUserOption(user);
              let hasPermission =
                !(Constants.foreverRoomIds |> Js.Array.includes(roomId))
                || userId == Some(adminUserId);
              switch (hasPermission, accessToken) {
              | (true, Some(accessToken)) =>
                let%Repromise contextTracks =
                  Spotify.getContextTracks(
                    ~accessToken,
                    ~contextType,
                    ~contextId,
                  );

                let now = Js.Date.now();
                let%Repromise (tracks, startTimestamp) =
                  switch (
                    contextTracks
                    |> Js.Array.findIndex((track: SpotifyTypes.track) =>
                         track.id == trackId
                       )
                  ) {
                  | (-1) =>
                    let%Repromise trackInfo =
                      Spotify.getTrackInfo(~accessToken, ~trackId);
                    Promise.resolved(([|trackInfo|], now));
                  | trackOffset =>
                    let beforeTrackDuration = ref(0.);
                    for (i in 0 to trackOffset - 1) {
                      beforeTrackDuration :=
                        beforeTrackDuration^
                        +. Option.getExn(contextTracks[i]).durationMs;
                    };
                    Promise.resolved((
                      contextTracks,
                      now -. beforeTrackDuration^,
                    ));
                  };

                let serializedRoomTracks =
                  tracks |> Js.Array.map(SocketMessage.serializeSpotifyTrack);

                let publishedUserId =
                  Option.getExn(user).anonymous
                    ? "anonymous" : Option.getExn(userId);
                // TODO: check roomId == storedRoomId
                {
                  let updatedRoom: Room.t = {
                    id: roomId,
                    record:
                      Some((
                        publishedUserId,
                        albumId,
                        serializedRoomTracks,
                        startTimestamp,
                      )),
                  };
                  let%Repromise client = Database.getClient();
                  let%Repromise _ = Database.updateRoom(client, updatedRoom);
                  Database.releaseClient(client);
                }
                |> ignore;

                io
                ->inRoom(roomId)
                ->Socket.emit(
                    StartRecord(
                      roomId,
                      publishedUserId,
                      albumId,
                      serializedRoomTracks,
                      startTimestamp,
                    ),
                  );
                Promise.resolved();
              | _ => Promise.resolved()
              };
              Promise.resolved();
            | None => Promise.resolved()
            }
          )
          |> ignore;
        | RemoveRecord(roomId) =>
          let roomId = Js.String.toLowerCase(Utils.sanitizeRoomId(roomId));
          switch (roomIdRef^) {
          | Some(roomId) =>
            {
              let%Repromise client = Database.getClient();
              let%Repromise room = Database.getRoom(client, roomId);
              let%Repromise _ =
                switch (room) {
                | Some(room) =>
                  let updatedRoom = {...room, record: None};
                  Database.updateRoom(client, updatedRoom);
                | None => Promise.resolved()
                };
              Database.releaseClient(client);
            }
            |> ignore;
            io->inRoom(roomId)->Socket.emit(RemoveRecord(roomId));
          // TODO: check roomId == storedRoomId
          | None => ()
          };
        | SetAnonymous(sessionId, roomId, anonymous) =>
          let roomId = Js.String.toLowerCase(Utils.sanitizeRoomId(roomId));
          switch (roomIdRef^) {
          | Some(roomId) =>
            {
              let%Repromise client = Database.getClient();
              let%Repromise session = Database.getSession(client, sessionId);
              let userId = Option.map(session, session => session.userId);
              let%Repromise user =
                switch (userId) {
                | Some(userId) => Database.getUser(client, userId)
                | None => Promise.resolved(None)
                };
              let%Repromise _ =
                switch (user) {
                | Some(user) =>
                  Database.updateUser(client, {...user, anonymous})
                | _ => Promise.resolved()
                };
              Database.releaseClient(client) |> ignore;
              Option.flatMap(
                Js.Dict.get(roomConnections, roomId), connections => {
                (
                  switch (anonymous, user) {
                  | (true, _) =>
                    Some(
                      {id: socketId, userId: "anonymous"}: SocketMessage.connection,
                    )
                  | (false, Some(user)) =>
                    Some({id: socketId, userId: user.id})
                  | (false, None) => None
                  }
                )
                ->Option.map(updatedConnection =>
                    (connections, updatedConnection)
                  )
              })
              ->Option.map(((connections, updatedConnection)) => {
                  roomConnections->Js.Dict.set(
                    roomId,
                    connections
                    |> Js.Array.map((connection: SocketMessage.connection) =>
                         connection.id != socketId
                           ? connection : updatedConnection
                       ),
                  );
                  io
                  ->inRoom(roomId)
                  ->Socket.emit(
                      UpdateConnection(
                        roomId,
                        SocketMessage.serializeConnection(updatedConnection),
                      ),
                    );
                })
              |> ignore;
              Promise.resolved();
            }
            |> ignore
          // TODO: check roomId == storedRoomId
          | None => ()
          };
        | Logout(roomId) =>
          let roomId = Js.String.toLowerCase(Utils.sanitizeRoomId(roomId));
          switch (Js.Dict.get(roomConnections, roomId)) {
          | Some(connections) =>
            roomConnections->Js.Dict.set(
              roomId,
              connections
              |> Js.Array.map((connection: SocketMessage.connection) =>
                   connection.id != socketId
                     ? connection : {id: socketId, userId: ""}
                 ),
            );

            io
            ->inRoom(roomId)
            ->Socket.emit(LogoutConnection(roomId, socketId));
          | None => ()
          };
        };
      },
    );

    socket->Socket.onDisconnect(() => {
      switch (roomIdRef^) {
      | Some(roomId) => removeRoomConnection(socket, roomId, socketId)
      | None => ()
      }
    });
  },
);