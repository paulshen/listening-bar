module Styles = {
  open Css;
  let root = style([media("(min-width: 1200px)", [display(flexBox)])]);
  let albumImage = style([width(px(320)), height(px(320))]);
  let albumImagePlaceholder = style([backgroundColor(hex("636363"))]);
  let albumInfo =
    style([
      textAlign(center),
      marginBottom(px(32)),
      marginTop(px(32)),
      width(px(320)),
    ]);
  let currentSpinning =
    style([
      fontSize(px(10)),
      letterSpacing(pxFloat(2.)),
      marginBottom(px(16)),
      textTransform(uppercase),
    ]);
  let albumNameRow = style([marginBottom(px(8))]);
  let albumName = style([fontSize(px(28)), fontWeight(`num(500))]);
  let trackArtist =
    style([
      fontSize(px(18)),
      marginRight(px(12)),
      lastChild([marginRight(zero)]),
    ]);
  let playlist =
    style([
      width(px(320)),
      fontSize(px(14)),
      media("(min-width: 1200px)", [marginLeft(px(64))]),
      media("(min-width: 1400px)", [marginLeft(px(128))]),
    ]);
  let noRecordPlaylistImage = style([width(px(320)), height(px(320))]);
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
      (
        ~track: SocketMessage.roomTrack,
        ~startTimestamp: option(float),
        ~onFinish,
      ) => {
    <div className=Styles.root>
      <div
        className={Cn.make([
          Styles.trackName,
          Cn.ifTrue(
            Styles.trackNamePlaying,
            Belt.Option.isSome(startTimestamp),
          ),
        ])}>
        <a
          href={"https://open.spotify.com/track/" ++ track.trackId}
          target="_blank">
          {React.string(track.trackName)}
        </a>
      </div>
      {switch (startTimestamp) {
       | Some(startTimestamp) =>
         <TrackPlaybar startTimestamp track onFinish />
       | None => React.null
       }}
    </div>;
  };
};

[@react.component]
let make =
    (
      ~roomTrackWithMetadata: option((SocketMessage.roomTrack, int, float)),
      ~roomRecord:
         option((string, string, array(SocketMessage.roomTrack), float)),
      ~isLoggedIn: bool,
      ~onTrackFinish,
    ) => {
  switch (roomTrackWithMetadata) {
  | Some((roomTrack, index, trackStartTimestamp)) =>
    let (userId, _albumId, playlistTracks, _) =
      Belt.Option.getExn(roomRecord);
    <div className=Styles.root>
      <div>
        <div>
          <img src={roomTrack.albumImage} className=Styles.albumImage />
        </div>
        <div className=Styles.albumInfo>
          <div className=Styles.currentSpinning>
            {React.string("Currently Spinning")}
          </div>
          <div className=Styles.albumNameRow>
            <a
              href={"https://open.spotify.com/album/" ++ roomTrack.albumId}
              target="_blank"
              className=Styles.albumName>
              {React.string(roomTrack.albumName)}
            </a>
          </div>
          <div>
            {roomTrack.artists
             |> Js.Array.map((artist: SocketMessage.roomTrackArtist) =>
                  <a
                    href={"https://open.spotify.com/artist/" ++ artist.id}
                    target="_blank"
                    className=Styles.trackArtist
                    key={artist.id}>
                    {React.string(artist.name)}
                  </a>
                )
             |> React.array}
          </div>
        </div>
      </div>
      <div className=Styles.playlist>
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
                onFinish=onTrackFinish
                key={track.trackId}
              />
            )
         |> React.array}
      </div>
    </div>;
  | None =>
    <div className=Styles.root>
      <div>
        <img
          src="/assets/drawing.png"
          className=Styles.noRecordPlaylistImage
        />
        <div className=Styles.albumInfo>
          <div className=Styles.currentSpinning>
            {React.string("It's quiet in here")}
          </div>
        </div>
      </div>
      <div className=Styles.playlist />
    </div>
  };
};