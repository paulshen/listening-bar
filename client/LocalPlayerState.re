[@react.component]
let make = () => {
  let {currentTrack, playerState, updateTimestamp}: SpotifyStore.state =
    SpotifyStore.useState();

  switch (currentTrack) {
  | Some((track, context)) =>
    <div>
      <div> {React.string(track.album.name)} </div>
      <div> {React.string(track.artists[0].name)} </div>
      <div> {React.string(track.name)} </div>
      <div> {React.string(string_of_int(track.trackNumber))} </div>
    </div>
  | None => React.null
  };
};