type t = {
  id: string,
  connections: array(SocketMessage.connection),
  playlist: option((array(SocketMessage.serializedRoomTrack), float)),
};