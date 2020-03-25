module Socket = BsSocket.Client.Make(SocketMessage);

let socketRef = ref(None);

let getSocket = () => {
  switch (socketRef^) {
  | Some(socket) => socket
  | None =>
    let socket = Socket.createWithUrl(Constants.serverUrl);
    socketRef := Some(socket);

    Socket.on(
      socket,
      message => {
        Js.log(message);
        switch (message) {
        | NewConnection(roomId, connection) =>
          RoomStore.addConnection(
            roomId,
            SocketMessage.deserializeConnection(connection),
          )
        | LostConnection(roomId, connectionId) =>
          RoomStore.removeConnection(roomId, connectionId)
        | LogoutConnection(roomId, connectionId) =>
          RoomStore.logoutConnection(roomId, connectionId)
        | Connected(roomId, connections, serializedTracks, startTimestamp) =>
          RoomStore.updateRoom({
            id: roomId,
            connections:
              connections |> Js.Array.map(SocketMessage.deserializeConnection),
            playlist:
              if (serializedTracks == [||]) {
                None;
              } else {
                Some((
                  serializedTracks
                  |> Js.Array.map(SocketMessage.deserializeRoomTrack),
                  startTimestamp,
                ));
              },
          });
          RoomStore.updateCurrentRoomId(Some(roomId));
        | PublishPlaylist(roomId, serializedTracks, startTimestamp) =>
          RoomStore.updatePlaylist(
            roomId,
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