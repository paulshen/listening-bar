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
        | Connected(roomId, connections, serializedTrack, startTimestamp) =>
          let (trackId, _, _, _, _, _, _, _) = serializedTrack;
          RoomStore.updateRoom({
            id: roomId,
            connections:
              connections |> Js.Array.map(SocketMessage.deserializeConnection),
            track:
              if (trackId == "") {
                None;
              } else {
                Some((
                  SocketMessage.deserializeRoomTrack(serializedTrack),
                  startTimestamp,
                ));
              },
          });
          RoomStore.updateCurrentRoomId(Some(roomId));
        | PublishTrackState(roomId, serializedTrack, startTimestamp) =>
          RoomStore.updateTrack(
            roomId,
            SocketMessage.deserializeRoomTrack(serializedTrack),
            startTimestamp,
          )
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

let publishCurrentTrack = (sessionId, trackId, startTimestamp) => {
  Socket.emit(
    getSocket(),
    PublishTrackState(
      sessionId,
      RoomStore.getCurrentRoomId()->Belt.Option.getExn,
      (trackId, startTimestamp),
    ),
  );
};

let logout = () => {
  switch (RoomStore.getCurrentRoomId()) {
  | Some(roomId) => Socket.emit(getSocket(), Logout(roomId))
  | _ => ()
  };
};