type trackState = (string, float);
type connection = {
  id: string,
  userId: string,
};
type roomTrack = {
  trackId: string,
  trackName: string,
  artistId: string,
  artistName: string,
  albumId: string,
  albumName: string,
  albumImage: string,
  durationMs: float,
};
type serializedRoomTrack = (
  string,
  string,
  string,
  string,
  string,
  string,
  string,
  float,
);

let serializeOptionTrackState = trackState => {
  switch (trackState) {
  | Some(trackState) =>
    let (trackId, startTimestamp) = trackState;
    [|trackId, Js.Float.toString(startTimestamp)|] |> Js.Array.joinWith(":");
  | None => ""
  };
};

let deserializeOptionTrackState = (input): option(trackState) => {
  switch (input) {
  | "" => None
  | input =>
    let pieces = input |> Js.String.split(":");
    Some((pieces[0], Js.Float.fromString(pieces[1])));
  };
};

let serializeSpotifyTrack = (track: SpotifyTypes.track): serializedRoomTrack => {
  (
    track.id,
    track.name,
    track.artists[0].id,
    track.artists[0].name,
    track.album.id,
    track.album.name,
    track.album.images[0].url,
    track.durationMs,
  );
};

let deserializeRoomTrack =
    (
      (
        trackId,
        trackName,
        artistId,
        artistName,
        albumId,
        albumName,
        albumImage,
        durationMs,
      ): serializedRoomTrack,
    )
    : roomTrack => {
  {
    trackId,
    trackName,
    artistId,
    artistName,
    albumId,
    albumName,
    albumImage,
    durationMs,
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
  | PublishTrackState(string, string, trackState);
type serverToClient =
  | Connected(string, array((string, string)), serializedRoomTrack, float)
  | NewConnection(string, (string, string))
  | LostConnection(string, string)
  | PublishTrackState(string, serializedRoomTrack, float);