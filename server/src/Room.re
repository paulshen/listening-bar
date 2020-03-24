type t = {
  id: string,
  connections: array(SocketMessage.connection),
  trackState: option(SocketMessage.trackState),
};