type trackState = (string, float, float);
type connection = {
  id: string,
  userId: string,
};

let serializeOptionTrackState = trackState => {
  switch (trackState) {
  | Some(trackState) =>
    let (trackId, startTimestamp, durationMs) = trackState;
    [|
      trackId,
      Js.Float.toString(startTimestamp),
      Js.Float.toString(durationMs),
    |]
    |> Js.Array.joinWith(":");
  | None => ""
  };
};

let deserializeOptionTrackState = input => {
  switch (input) {
  | "" => None
  | input =>
    let pieces = input |> Js.String.split(":");
    Some((
      pieces[0],
      Js.Float.fromString(pieces[1]),
      Js.Float.fromString(pieces[2]),
    ));
  };
};

let serializeConnection = (connection: connection) => {
  (connection.id, connection.userId);
};
let deserializeConnection = ((id, userId)) => {
  {id, userId};
};

type clientToServer =
  | JoinRoom(string, string)
  | PublishTrackState(string, trackState);
type serverToClient =
  | Connected(string, array((string, string)), string)
  | NewConnection(string, (string, string))
  | LostConnection(string, string)
  | PublishTrackState(string, trackState);