type trackState = (string, string, string, float);
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
    let (trackId, contextType, contextId, startTimestamp) = trackState;
    [|trackId, contextType, contextId, Js.Float.toString(startTimestamp)|]
    |> Js.Array.joinWith(":");
  | None => ""
  };
};

let deserializeOptionTrackState = (input): option(trackState) => {
  switch (input) {
  | "" => None
  | input =>
    let pieces = input |> Js.String.split(":");
    Some((pieces[0], pieces[1], pieces[2], Js.Float.fromString(pieces[3])));
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
  | PublishTrackState(string, string, trackState)
  | RemoveRecord(string)
  | Logout(string);
type serverToClient =
  | Connected(
      string,
      array((string, string)),
      array(serializedRoomTrack),
      float,
    )
  | NewConnection(string, (string, string))
  | LostConnection(string, string)
  | LogoutConnection(string, string)
  | PublishPlaylist(string, array(serializedRoomTrack), float)
  | RemoveRecord(string);