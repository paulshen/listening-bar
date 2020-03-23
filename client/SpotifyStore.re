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
  album,
  artists: array(artist),
};

type state = {
  currentTrack: option(track),
  isPlaying: bool,
};
type action =
  | UpdateState(option(track), bool);

let api =
  Restorative.createStore(
    {currentTrack: None, isPlaying: false}, (state, action) => {
    switch (action) {
    | UpdateState(track, isPlaying) => {currentTrack: track, isPlaying}
    }
  });

let useState = api.useStore;
let updateState = (track, isPlaying) =>
  api.dispatch(UpdateState(track, isPlaying));