module Socket = BsSocket.Client.Make(SocketMessage);

let socket = Socket.createWithUrl("http://localhost:3000");

Socket.on(
  socket,
  message => {
    Js.log(message);
    switch (message) {
    | NewUser(roomId, userId) => RoomStore.addUser(roomId, userId)
    | Connected(roomId, userIds, serializedTrackState) =>
      RoomStore.updateRoom({
        id: roomId,
        userIds,
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