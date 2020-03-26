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

let getCurrentTrack =
    (trackDurations: array(float), startTimestamp: float)
    : option((int, float)) => {
  let result = ref(None);
  let i = ref(0);
  let now = Js.Date.now();
  let timestamp = ref(startTimestamp);
  while (Belt.Option.isNone(result^) && i^ < Js.Array.length(trackDurations)) {
    let durationMs = trackDurations[i^];
    let songEnd = timestamp^ +. durationMs;
    if (now < songEnd) {
      result := Some((i^, timestamp^));
    } else {
      i := i^ + 1;
      timestamp := songEnd;
    };
  };
  result^;
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
      string,
      string,
      array(serializedRoomTrack),
      float,
    )
  | NewConnection(string, (string, string))
  | LostConnection(string, string)
  | LogoutConnection(string, string)
  | StartRecord(string, string, array(serializedRoomTrack), float)
  | RemoveRecord(string);