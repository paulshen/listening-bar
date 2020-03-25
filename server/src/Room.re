type t = {
  id: string,
  connections: array(SocketMessage.connection),
  record: option((string, array(SocketMessage.serializedRoomTrack), float)),
};