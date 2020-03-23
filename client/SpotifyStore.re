type artist = {
  id: string,
  name: string,
};
type albumImage = {
  url: string,
  width: int,
  height: int,
};
type album = {
  id: string,
  images: array(albumImage),
  name: string,
};
type track = {
  id: string,
  name: string,
  uri: string,
  durationMs: float,
  album,
  artists: array(artist),
};

type playerState =
  | NotPlaying
  | Playing(float);

type state = {
  currentTrack: option(track),
  playerState,
};
type action =
  | UpdateState(option(track), playerState);

let api =
  Restorative.createStore(
    {currentTrack: None, playerState: NotPlaying}, (state, action) => {
    switch (action) {
    | UpdateState(track, playerState) => {currentTrack: track, playerState}
    }
  });

let useState = api.useStore;
let updateState = (track, playerState) =>
  api.dispatch(UpdateState(track, playerState));