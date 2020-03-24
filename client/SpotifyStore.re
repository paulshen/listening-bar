type playerState =
  | NotPlaying
  | Playing(float);

type state = {
  currentTrack: option(SpotifyTypes.track),
  playerState,
};
type action =
  | UpdateState(option(SpotifyTypes.track), playerState);

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