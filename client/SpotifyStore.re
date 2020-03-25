type playerState =
  | NotPlaying
  | Playing(float);

type state = {
  currentTrack: option((SpotifyTypes.track, SpotifyTypes.context)),
  playerState,
  updateTimestamp: float,
};
type action =
  | UpdateState(
      option((SpotifyTypes.track, SpotifyTypes.context)),
      playerState,
      float,
    );

let api =
  Restorative.createStore(
    {currentTrack: None, playerState: NotPlaying, updateTimestamp: 0.},
    (state, action) => {
    switch (action) {
    | UpdateState(track, playerState, timestamp) => {
        currentTrack: track,
        playerState,
        updateTimestamp: timestamp,
      }
    }
  });

let useState = api.useStore;
let updateState = (track, playerState, timestamp) =>
  api.dispatch(UpdateState(track, playerState, timestamp));

let fetch = () => {
  let%Repromise result = SpotifyClient.getCurrentTrack();
  switch (result) {
  | Some((track, context, isPlaying, startTimestamp)) =>
    let playerState = isPlaying ? Playing(startTimestamp) : NotPlaying;
    updateState(Some((track, context)), playerState, Js.Date.now());
    Promise.resolved(Some((track, context, playerState)));
  | None =>
    updateState(None, NotPlaying, Js.Date.now());
    Promise.resolved(None);
  };
};

let fetchIfNeeded = (~bufferMs) => {
  let {currentTrack, playerState, updateTimestamp} = api.getState();
  let now = Js.Date.now();
  if (now -. updateTimestamp < float_of_int(bufferMs)) {
    Belt.Option.map(currentTrack, ((track, context)) => {
      (track, context, playerState)
    })
    |> Promise.resolved;
  } else {
    fetch();
  };
};