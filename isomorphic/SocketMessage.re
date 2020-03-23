type trackState = (string, float, float);

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

type clientToServer =
  | JoinRoom(string, string)
  | PublishTrackState(string, trackState);
type serverToClient =
  | Connected(string, array(string), string)
  | NewUser(string, string)
  | PublishTrackState(string, trackState);