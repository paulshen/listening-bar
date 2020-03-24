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
    | Connected(roomId, connections, serializedTrackState) =>
      RoomStore.updateRoom({
        id: roomId,
        connections:
          connections |> Js.Array.map(SocketMessage.deserializeConnection),
        trackState:
          SocketMessage.deserializeOptionTrackState(serializedTrackState),
      });
      RoomStore.updateCurrentRoomId(Some(roomId));
    | PublishTrackState(roomId, trackState) =>
      RoomStore.updateTrackState(roomId, trackState)
    };
  },
);

let connectToRoom = (roomId, sessionId) => {
  Socket.emit(socket, JoinRoom(roomId, sessionId));
};

let publishCurrentTrack = (trackId, startTimestamp, durationMs) => {
  Socket.emit(
    socket,
    PublishTrackState(
      RoomStore.getCurrentRoomId()->Belt.Option.getExn,
      (trackId, startTimestamp, durationMs),
    ),
  );
};