module Socket = BsSocket.Client.Make(SocketMessage);

let socket = Socket.createWithUrl(Constants.serverUrl);

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

let connectToRoom = (roomId, sessionId) => {
  Socket.emit(socket, JoinRoom(roomId, sessionId));
};

let publishCurrentTrack = (sessionId, trackId, startTimestamp) => {
  Socket.emit(
    socket,
    PublishTrackState(
      sessionId,
      RoomStore.getCurrentRoomId()->Belt.Option.getExn,
      (trackId, startTimestamp),
    ),
  );
};