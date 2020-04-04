module Socket = BsSocket.Client.Make(SocketMessage);

let socketRef = ref(None);

[@bs.get] external getSocketId: Socket.t => string = "id";

let getSocket = () => {
  switch (socketRef^) {
  | Some(socket) => socket
  | None =>
    let socket = Socket.createWithUrl(Constants.serverUrl);
    socketRef := Some(socket);

    Socket.on(
      socket,
      message => {
        if (!Constants.isProduction) {
          Js.log(message);
        };
        switch (message) {
        | NewConnection(roomId, connection) =>
          RoomStore.addConnection(
            roomId,
            SocketMessage.deserializeConnection(connection),
          )
        | UpdateConnection(roomId, connection) =>
          let connection = SocketMessage.deserializeConnection(connection);
          if (connection.id == socket->getSocketId) {
            UserStore.setAnonymous(connection.userId == "anonymous");
          };
          RoomStore.updateConnection(roomId, connection);
        | LostConnection(roomId, connectionId) =>
          RoomStore.removeConnection(roomId, connectionId)
        | LogoutConnection(roomId, connectionId) =>
          RoomStore.logoutConnection(roomId, connectionId)
        | Connected(
            roomId,
            connections,
            userId,
            albumId,
            serializedTracks,
            startTimestamp,
          ) =>
          RoomStore.updateRoom({
            id: roomId,
            connections:
              connections |> Js.Array.map(SocketMessage.deserializeConnection),
            record:
              if (serializedTracks == [||]) {
                None;
              } else {
                Some((
                  userId,
                  albumId,
                  serializedTracks
                  |> Js.Array.map(SocketMessage.deserializeRoomTrack),
                  startTimestamp,
                ));
              },
          });
          RoomStore.updateCurrentRoomId(Some(roomId));
        | StartRecord(
            roomId,
            userId,
            albumId,
            serializedTracks,
            startTimestamp,
          ) =>
          RoomStore.startRecord(
            roomId,
            userId,
            albumId,
            serializedTracks
            |> Js.Array.map(SocketMessage.deserializeRoomTrack),
            startTimestamp,
          )
        | RemoveRecord(roomId) => RoomStore.removeRecord(roomId)
        };
      },
    );

    socket;
  };
};

let connectToRoom = (roomId, sessionId) => {
  Socket.emit(
    getSocket(),
    JoinRoom(roomId, Belt.Option.getWithDefault(sessionId, "")),
  );
};

let publishCurrentTrack =
    (sessionId, trackId, contextType, contextId, startTimestamp) => {
  Socket.emit(
    getSocket(),
    PublishTrackState(
      sessionId,
      RoomStore.getCurrentRoomId()->Belt.Option.getExn,
      (trackId, contextType, contextId, startTimestamp),
    ),
  );
};

let removeRecord = roomId => {
  Socket.emit(getSocket(), RemoveRecord(roomId));
};

let logout = () => {
  switch (RoomStore.getCurrentRoomId()) {
  | Some(roomId) => Socket.emit(getSocket(), Logout(roomId))
  | _ => ()
  };
};

let setAnonymous = (sessionId, roomId, anonymous) => {
  Socket.emit(getSocket(), SetAnonymous(sessionId, roomId, anonymous));
};