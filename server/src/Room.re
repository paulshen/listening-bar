type t = {
  id: string,
  record:
    option(
      (string, string, array(SocketMessage.serializedRoomTrack), float),
    ),
};