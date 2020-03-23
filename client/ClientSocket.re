module Socket = BsSocket.Client.Make(SocketMessage);

let socket = Socket.createWithUrl("http://localhost:3000");

Socket.on(
  socket,
  message => {
    Js.log(message);
    switch (message) {
    | NewUser(roomId, userId) => RoomStore.addUser(roomId, userId)
    | Connected(roomId, userIds) =>
      RoomStore.updateRoom({id: roomId, userIds})
    };
  },
);

let connectToRoom = (roomId, sessionId) => {
  Socket.emit(socket, JoinRoom(roomId, sessionId));
};