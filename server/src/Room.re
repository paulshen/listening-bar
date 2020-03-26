type t = {
  id: string,
  connections: array(SocketMessage.connection),
  record:
    option(
      (string, string, array(SocketMessage.serializedRoomTrack), float),
    ),
};