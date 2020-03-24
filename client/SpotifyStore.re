type playerState =
  | NotPlaying
  | Playing(float);

type state = {
  currentTrack: option((SpotifyTypes.track, SpotifyTypes.context)),
  playerState,
};
type action =
  | UpdateState(
      option((SpotifyTypes.track, SpotifyTypes.context)),
      playerState,
    );

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