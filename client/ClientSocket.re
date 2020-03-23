module Socket = BsSocket.Client.Make(SocketMessage);

let socket = Socket.createWithUrl("http://localhost:3000");

Socket.on(
  socket,
  message => {
    Js.log(message);
    switch (message) {
    | NewUser(roomId, userId) => RoomStore.addUser(roomId, userId)
    | Connected(room) => RoomStore.updateRoom(room)
    };
  },
);

let connectToRoom = (roomId, sessionId) => {
  Socket.emit(socket, JoinRoom(roomId, sessionId));
};