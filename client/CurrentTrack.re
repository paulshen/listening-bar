module Styles = {
  open Css;
  let trackName = style([fontSize(px(24))]);
  let trackArtist = style([fontSize(px(18))]);
};

[@react.component]
let make = (~roomTrack: SocketMessage.roomTrack, ~startTimestamp: float) => {
  let (hasEnded, setHasEnded) =
    React.useState(() => {
      roomTrack.durationMs +. startTimestamp < Js.Date.now()
    });

  React.useEffect0(() =>
    if (!hasEnded) {
      let timeout =
        Js.Global.setTimeoutFloat(
          () => {setHasEnded(_ => true)},
          roomTrack.durationMs +. startTimestamp -. Js.Date.now(),
        );
      Some(() => Js.Global.clearTimeout(timeout));
    } else {
      None;
    }
  );

  if (!hasEnded) {
    <div>
      <div>
        <img
          src={roomTrack.albumImage}
          style={ReactDOMRe.Style.make(~width="256px", ~height="256px", ())}
        />
      </div>
      <div>
        <a
          href={"https://open.spotify.com/track/" ++ roomTrack.trackId}
          className=Styles.trackName>
          {React.string(roomTrack.trackName)}
        </a>
      </div>
      <div>
        <a
          href={"https://open.spotify.com/artist/" ++ roomTrack.artistId}
          className=Styles.trackArtist>
          {React.string(roomTrack.artistName)}
        </a>
      </div>
      <div> <TrackPlaybar startTimestamp roomTrack /> </div>
    </div>;
  } else {
    React.null;
  };
};