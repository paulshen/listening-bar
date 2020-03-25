module Styles = {
  open Css;
  let root = style([width(px(320))]);
  let albumImage = style([width(px(320)), height(px(320))]);
  let albumInfo =
    style([textAlign(center), marginBottom(px(32)), marginTop(px(16))]);
  let currentSpinning =
    style([
      fontSize(px(10)),
      letterSpacing(pxFloat(2.)),
      marginBottom(px(16)),
      textTransform(uppercase),
    ]);
  let albumNameRow = style([marginBottom(px(4))]);
  let albumName = style([fontSize(px(24)), fontWeight(`num(500))]);
  let trackArtist = style([fontSize(px(18))]);
};

module TrackRow = {
  module Styles = {
    open Css;
    let root =
      style([
        borderTop(px(1), solid, rgba(253, 254, 195, 0.2)),
        padding2(~v=px(6), ~h=zero),
        display(flexBox),
        alignItems(center),
      ]);
    let trackName =
      style([
        flexGrow(1.),
        fontSize(px(14)),
        whiteSpace(nowrap),
        overflow(hidden),
        textOverflow(ellipsis),
      ]);
    let trackNamePlaying = style([fontWeight(`num(600))]);
  };

  [@react.component]
  let make =
      (~track: SocketMessage.roomTrack, ~startTimestamp: option(float)) => {
    <div className=Styles.root>
      <div
        className={Cn.make([
          Styles.trackName,
          Cn.ifTrue(
            Styles.trackNamePlaying,
            Belt.Option.isSome(startTimestamp),
          ),
        ])}>
        {React.string(track.trackName)}
      </div>
      {switch (startTimestamp) {
       | Some(startTimestamp) => <TrackPlaybar startTimestamp track />
       | None => React.null
       }}
    </div>;
  };
};

[@react.component]
let make =
    (
      ~roomPlaylist: (array(SocketMessage.roomTrack), float),
      ~roomTrack: SocketMessage.roomTrack,
      ~index: int,
      ~trackStartTimestamp: float,
    ) => {
  let (playlistTracks, _) = roomPlaylist;
  <div className=Styles.root>
    <div> <img src={roomTrack.albumImage} className=Styles.albumImage /> </div>
    <div className=Styles.albumInfo>
      <div className=Styles.currentSpinning>
        {React.string("Currently Spinning")}
      </div>
      <div className=Styles.albumNameRow>
        <a
          href={"https://open.spotify.com/album/" ++ roomTrack.albumId}
          className=Styles.albumName>
          {React.string(roomTrack.albumName)}
        </a>
      </div>
      <div>
        <a
          href={"https://open.spotify.com/artist/" ++ roomTrack.artistId}
          className=Styles.trackArtist>
          {React.string(roomTrack.artistName)}
        </a>
      </div>
    </div>
    <div>
      {playlistTracks
       |> Js.Array.mapi((track: SocketMessage.roomTrack, i) =>
            <TrackRow
              track
              startTimestamp={
                               if (i == index) {
                                 Some(trackStartTimestamp);
                               } else {
                                 None;
                               }
                             }
              key={track.trackId}
            />
          )
       |> React.array}
    </div>
  </div>;
};