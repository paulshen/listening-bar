open Belt;
open Express;

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
  let%Repromise (sessionId, accessToken) = Spotify.getToken(code);
  let responseJson =
    Js.Json.object_(
      Js.Dict.fromArray([|
        ("sessionId", Js.Json.string(sessionId)),
        ("accessToken", Js.Json.string(accessToken)),
      |]),
    );
  res |> Response.sendJson(responseJson) |> Promise.resolved;
});

let getAccessTokenForUserId = userId => {
  let user = Persist.getUser(userId);
  let%Repromise accessToken =
    switch ((user: option(User.t))) {
    | Some(user) =>
      if (user.tokenExpireTime < Js.Date.now()) {
        let%Repromise (accessToken, tokenExpireTime) =
          Spotify.refreshToken(user.refreshToken);
        Persist.updateUser({
          ...user,
          id: userId,
          accessToken,
          tokenExpireTime,
        });
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
  Middleware.from((next, req, res) => {
    switch (
      Request.get("Authorization", req)->Option.flatMap(Persist.getSession)
    ) {
    | Some(session) =>
      setProperty(req, "session", Obj.magic(session), res) |> ignore;
      next(Next.middleware, res);
    | None => Response.sendStatus(Response.StatusCode.Unauthorized, res)
    }
  }),
  promiseMiddleware((next, req, res) => {
    let userId =
      Obj.magic(Option.getExn(getProperty(req, "session")))##userId;
    let%Repromise accessToken = getAccessTokenForUserId(userId);
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
        |]),
      );
    res |> Response.sendJson(responseJson) |> Promise.resolved;
  }),
|];

App.useRouterOnPath(app, ~path="/api", apiRouter);

let dirname: option(string) = [%bs.node __dirname];
App.get(app, ~path="/index.js") @@
Middleware.from((next, req, res) => {
  Response.sendFile(
    Node.Path.dirname(Node.Path.dirname(Option.getExn(dirname)))
    ++ "/build/index.js",
    Js.Dict.empty(),
    res,
  )
});
App.get(app, ~path="/*") @@
Middleware.from((next, req, res) => {
  Response.sendFile(
    Node.Path.dirname(Node.Path.dirname(Option.getExn(dirname)))
    ++ "/build/index.html",
    Js.Dict.empty(),
    res,
  )
});

let onListen = e =>
  switch (e) {
  | exception (Js.Exn.Error(e)) =>
    Js.log(e);
    Node.Process.exit(1);
  | _ => Js.log @@ "Listening at http://127.0.0.1:3030"
  };

let server = App.listen(app, ~port=3030, ~onListen, ());

let rooms: Js.Dict.t(Room.t) = Js.Dict.empty();

let io = SocketServer.createWithHttp(server);
Router.get(apiRouter, ~path="/rooms/:roomId") @@
Middleware.from((next, req, res) => {
  let roomId =
    Request.params(req)
    ->Js.Dict.unsafeGet("roomId")
    ->Js.Json.decodeString
    ->Option.getExn;
  let room = rooms->Js.Dict.get(roomId);
  switch (room) {
  | Some(room) =>
    res
    |> Response.sendJson(
         Js.Json.array(
           room.connections
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

SocketServer.onConnect(
  io,
  socket => {
    open SocketServer;
    let socketId = Socket.getId(socket);
    let roomIdRef = ref(None);
    Socket.on(
      socket,
      message => {
        Js.log(message);
        switch (message) {
        | JoinRoom(roomId, sessionId) =>
          socket->Socket.join(roomId) |> ignore;
          let session: option(User.session) =
            if (sessionId == "") {
              None;
            } else {
              Persist.getSession(sessionId);
            };
          let connection: SocketMessage.connection = {
            id: socketId,
            userId:
              switch (session) {
              | Some(session) => session.userId
              | None => ""
              },
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
            Option.getWithDefault(
              rooms->Js.Dict.get(roomId),
              {id: roomId, connections: [||], track: None},
            );
          let updatedRoom =
            if (room.connections
                |> Js.Array.findIndex((connection: SocketMessage.connection) =>
                     connection.id == socketId
                   )
                == (-1)) {
              {
                ...room,
                connections:
                  room.connections |> Js.Array.concat([|connection|]),
              };
            } else {
              room;
            };
          rooms->Js.Dict.set(roomId, updatedRoom);
          socket->Socket.emit(
            Connected(
              roomId,
              updatedRoom.connections
              |> Js.Array.map(SocketMessage.serializeConnection),
              switch (updatedRoom.track) {
              | None => ("", "", "", "", "", "", "", 0.)
              | Some((serializedTrack, _)) => serializedTrack
              },
              switch (updatedRoom.track) {
              | None => 0.
              | Some((_, startTimestamp)) => startTimestamp
              },
            ),
          );
          roomIdRef := Some(roomId);
        | PublishTrackState(sessionId, roomId, trackState) =>
          (
            switch (roomIdRef^) {
            | Some(storedRoomId) =>
              let (trackId, startTimestamp) = trackState;
              let%Repromise accessToken =
                Persist.getSession(sessionId)
                ->Option.map(session =>
                    getAccessTokenForUserId(session.userId)
                  )
                ->Option.getWithDefault(Promise.resolved(None));
              switch (accessToken) {
              | Some(accessToken) =>
                let%Repromise track =
                  Spotify.getTrackInfo(~accessToken, ~trackId);
                let serializedRoomTrack =
                  SocketMessage.serializeSpotifyTrack(track);

                // TODO: check roomId == storedRoomId
                switch (Js.Dict.get(rooms, roomId)) {
                | Some(room) =>
                  let updatedRoom = {
                    ...room,
                    track: Some((serializedRoomTrack, startTimestamp)),
                  };
                  rooms->Js.Dict.set(roomId, updatedRoom);
                  Persist.updateRoom(updatedRoom);
                | None => ()
                };

                io
                ->inRoom(roomId)
                ->Socket.emit(
                    PublishTrackState(
                      roomId,
                      serializedRoomTrack,
                      startTimestamp,
                    ),
                  );
                Promise.resolved();
              | None => Promise.resolved()
              };
            | None => Promise.resolved()
            }
          )
          |> ignore
        };
      },
    );

    socket->Socket.onDisconnect(() => {
      switch (roomIdRef^) {
      | Some(roomId) =>
        switch (Js.Dict.get(rooms, roomId)) {
        | Some(room) =>
          socket
          ->SocketServer.Socket.to_(roomId)
          ->Socket.emit(LostConnection(roomId, socketId));
          let updatedRoom = {
            ...room,
            connections:
              room.connections
              |> Js.Array.filter((connection: SocketMessage.connection) =>
                   connection.id != socketId
                 ),
          };
          rooms->Js.Dict.set(roomId, updatedRoom);
          Persist.updateRoom(updatedRoom);
        | None => ()
        }
      | None => ()
      }
    });
  },
);