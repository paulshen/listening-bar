type t = {
  id: string,
  connections: array(SocketMessage.connection),
  track: option((SocketMessage.serializedRoomTrack, float)),
};