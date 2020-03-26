module Styles = {
  open Css;
  let root =
    style([display(flexBox), justifyContent(center), paddingTop(px(64))]);
  let paneSpacer =
    style([
      width(px(128)),
      media("(min-width: 1200px)", [width(px(64))]),
      media("(min-width: 1400px)", [width(px(128))]),
    ]);
  let recordPlayer = style([marginBottom(px(32))]);
  let roomLabel =
    style([
      fontSize(px(10)),
      letterSpacing(pxFloat(2.)),
      textTransform(uppercase),
    ]);
  let roomName = style([fontSize(px(24)), marginBottom(px(32))]);
  let syncStatus = style([marginBottom(px(12))]);
  let syncRow =
    style([display(flexBox), alignItems(center), marginBottom(px(32))]);
  let manualSync =
    style([
      fontSize(px(12)),
      textTransform(uppercase),
      opacity(0.5),
      hover([opacity(1.)]),
      marginLeft(px(16)),
    ]);
};

let syncBuffer = 3000.;
let syncInterval = 30000;

let syncSpotify =
    (~roomTrackWithMetadata: (SocketMessage.roomTrack, int, float), ~force) => {
  let (track, index, startTimestamp) = roomTrackWithMetadata;
  let%Repromise skipSyncing =
    if (force) {
      Promise.resolved(false);
    } else {
      let%Repromise spotifyState = SpotifyStore.fetch();
      (
        switch (spotifyState) {
        | Some((localTrack, localContext, playerState)) =>
          switch (playerState) {
          | Playing(localStartTimestamp) =>
            localTrack.id === track.trackId
            && localContext.type_ === "album"
            && localContext.id === track.albumId
            && Js.Math.abs_float(localStartTimestamp -. startTimestamp)
            < syncBuffer
          | NotPlaying => false
          }
        | None => false
        }
      )
      |> Promise.resolved;
    };
  if (!skipSyncing) {
    let positionMs = Js.Date.now() -. startTimestamp;
    SpotifyClient.playAlbum(track.albumId, index, positionMs) |> ignore;
  };
  Promise.resolved();
};

module SpotifyStatePreview = {
  module Styles = {
    open Css;
    let root =
      style([
        backgroundColor(rgba(0, 0, 0, 0.9)),
        display(flexBox),
        fontSize(px(14)),
        padding(px(16)),
        boxSizing(borderBox),
        minWidth(px(288)),
        maxWidth(px(320)),
      ]);
    let spacer = style([width(px(24))]);
    let albumImage =
      style([width(px(96)), height(px(96)), flexShrink(0.)]);
    let albumName = style([fontWeight(`num(600))]);
  };

  [@react.component]
  let make = () => {
    let {currentTrack}: SpotifyStore.state = SpotifyStore.useState();
    switch (currentTrack) {
    | Some((track, context)) =>
      <div className=Styles.root>
        <img src={track.album.images[0].url} className=Styles.albumImage />
        <div className=Styles.spacer />
        <div>
          <div className=Styles.albumName>
            {React.string(track.album.name)}
          </div>
          <div> {React.string(track.artists[0].name)} </div>
          <div> {React.string(track.name)} </div>
        </div>
      </div>
    | None =>
      <div className=Styles.root>
        {React.string("First play a song on Spotify")}
      </div>
    };
  };
};

module ControlButtons = {
  module Styles = {
    open Css;
    let controlButtons =
      style([
        display(flexBox),
        alignItems(center),
        justifyContent(spaceBetween),
        marginBottom(px(32)),
      ]);
    let removeRecord =
      style([
        fontSize(px(12)),
        textTransform(uppercase),
        opacity(0.5),
        hover([opacity(1.)]),
      ]);
  };

  [@react.component]
  let make = (~roomId, ~hasRecordPlaying) => {
    let buttonRef = React.useRef(Js.Nullable.null);
    let (showPreview, setShowPreview) = React.useState(() => false);
    let onMouseEnter = _ => {
      SpotifyStore.fetchIfNeeded(~bufferMs=10000) |> ignore;
      setShowPreview(_ => true);
    };
    let onMouseLeave = _ => setShowPreview(_ => false);
    <div className=Styles.controlButtons>
      <Button
        onClick={_ => {
          {
            let%Repromise currentTrack =
              SpotifyStore.fetchIfNeeded(~bufferMs=10000);
            switch (currentTrack) {
            | Some((track, _context, _)) =>
              ClientSocket.publishCurrentTrack(
                UserStore.getSessionId()->Belt.Option.getExn,
                track.id,
                // always publish track's album
                "album",
                track.album.id,
                Js.Date.now(),
              )
            | _ => ()
            };
            Promise.resolved();
          }
          |> ignore
        }}
        onMouseEnter
        onMouseLeave
        domRef={ReactDOMRe.Ref.domRef(buttonRef)}>
        {React.string(hasRecordPlaying ? "Change Record" : "Put on Record")}
      </Button>
      {hasRecordPlaying
         ? <a
             href="#"
             onClick={e => {
               ReactEvent.Mouse.preventDefault(e);
               ClientSocket.removeRecord(roomId) |> ignore;
             }}
             className=Styles.removeRecord>
             {React.string("Remove Record")}
           </a>
         : React.null}
      {showPreview
         ? <ReactAtmosphere.PopperLayer
             reference=buttonRef
             render={_ => <SpotifyStatePreview />}
             options={
               placement: Some("top-start"),
               modifiers:
                 Some([|
                   {
                     "name": "offset",
                     "options": {
                       "offset": [|(-16), 8|],
                     },
                   },
                 |]),
             }
           />
         : React.null}
    </div>;
  };
};

[@react.component]
let make = (~roomId: string) => {
  let user = UserStore.useUser();
  let room = RoomStore.useRoom(roomId);
  let (isSyncing, setIsSyncing) = React.useState(() => false);

  if (Belt.Option.isNone(user) && isSyncing) {
    setIsSyncing(_ => false);
  };

  React.useEffect0(() => {
    ClientSocket.connectToRoom(roomId, UserStore.getSessionId());
    None;
  });

  let (_, forceUpdate) = React.useState(() => 1);
  let roomRecord = Belt.Option.flatMap(room, room => room.record);
  let roomTrackWithMetadata =
    Belt.Option.flatMap(roomRecord, ((_albumId, tracks, startTimestamp)) => {
      Belt.Option.map(
        SocketMessage.getCurrentTrack(
          tracks
          |> Js.Array.map((track: SocketMessage.roomTrack) =>
               track.durationMs
             ),
          startTimestamp,
        ),
        ((trackIndex, trackStartTimestamp)) => {
        (tracks[trackIndex], trackIndex, trackStartTimestamp)
      })
    });
  let roomTrackWithMetadataRef = React.useRef(roomTrackWithMetadata);
  React.useEffect1(
    () => {
      React.Ref.setCurrent(roomTrackWithMetadataRef, roomTrackWithMetadata);
      None;
    },
    [|roomTrackWithMetadata|],
  );
  React.useEffect2(
    () => {
      switch (roomRecord) {
      | Some(_) =>
        if (isSyncing) {
          syncSpotify(
            ~roomTrackWithMetadata=Belt.Option.getExn(roomTrackWithMetadata),
            ~force=true,
          )
          |> ignore;
        }
      | None =>
        if (isSyncing) {
          SpotifyClient.pausePlayer() |> ignore;
        }
      };
      None;
    },
    (roomRecord, isSyncing),
  );
  React.useEffect1(
    () =>
      if (isSyncing) {
        SpotifyClient.turnOffRepeat() |> ignore;
        let interval =
          Js.Global.setInterval(
            () => {
              switch (React.Ref.current(roomTrackWithMetadataRef)) {
              | Some(roomTrackWithMetadata) =>
                syncSpotify(~roomTrackWithMetadata, ~force=false) |> ignore
              | None => ()
              }
            },
            syncInterval,
          );
        Some(() => {Js.Global.clearInterval(interval)});
      } else {
        None;
      },
    [|isSyncing|],
  );

  // TODO: move UI update into <CurrentRecord>
  let roomTrackId =
    Belt.Option.map(roomTrackWithMetadata, ((track, _, _)) => track.trackId);
  React.useEffect1(
    () =>
      switch (roomTrackId) {
      | Some(_) =>
        let (roomTrack, index, startTimestamp) =
          Belt.Option.getExn(roomTrackWithMetadata);
        let songEnd = startTimestamp +. roomTrack.durationMs;
        let timeout =
          Js.Global.setTimeoutFloat(
            () => {forceUpdate(x => x + 1)},
            songEnd -. Js.Date.now(),
          );
        Some(() => Js.Global.clearTimeout(timeout));
      | None => None
      },
    [|roomTrackId|],
  );

  <div className=Styles.root>
    <div>
      <RecordPlayer
        startTimestamp={Belt.Option.map(roomRecord, ((_, _, startTimestamp)) =>
          startTimestamp
        )}
        totalDuration={Belt.Option.map(
          roomRecord,
          ((_, tracks, _)) => {
            let duration = ref(0.);
            tracks
            |> Js.Array.forEach((track: SocketMessage.roomTrack) => {
                 duration := duration^ +. track.durationMs
               });
            duration^;
          },
        )}
        className=Styles.recordPlayer
      />
      <div className=Styles.roomLabel> {React.string("Room")} </div>
      <div className=Styles.roomName> {React.string(roomId)} </div>
      {switch (user) {
       | Some(_user) =>
         <div>
           <div className=Styles.syncStatus>
             {isSyncing
                ? <>
                    <div> {React.string("Your Spotify is synced.")} </div>
                    <div>
                      {React.string(
                         "If you leave this page, you will stop syncing.",
                       )}
                    </div>
                  </>
                : <div> {React.string("Your Spotify is not synced.")} </div>}
           </div>
           <div className=Styles.syncRow>
             {isSyncing
                ? <Button onClick={_ => setIsSyncing(_ => false)}>
                    {React.string("Stop Sync")}
                  </Button>
                : <Button onClick={_ => setIsSyncing(_ => true)}>
                    {React.string("Start Sync")}
                  </Button>}
             {switch (roomTrackWithMetadata) {
              | Some(roomTrackWithMetadata) =>
                <a
                  href="#"
                  onClick={e => {
                    ReactEvent.Mouse.preventDefault(e);
                    syncSpotify(~roomTrackWithMetadata, ~force=false)
                    |> ignore;
                  }}
                  className=Styles.manualSync>
                  {React.string("Manual Sync")}
                </a>
              | None => React.null
              }}
           </div>
           <ControlButtons
             roomId
             hasRecordPlaying={Belt.Option.isSome(roomTrackWithMetadata)}
           />
         </div>
       | None =>
         <div>
           <Button
             onClick={e => {
               ReactEvent.Mouse.preventDefault(e);
               API.clientLogin();
             }}>
             {React.string("Login")}
           </Button>
         </div>
       }}
      {switch (room) {
       | Some(room) =>
         <div>
           <div>
             {React.string(
                string_of_int(Js.Array.length(room.connections))
                ++ " connected",
              )}
           </div>
           <div>
             {room.connections
              |> Js.Array.map((connection: SocketMessage.connection) =>
                   <div key={connection.id}>
                     {React.string(connection.userId)}
                   </div>
                 )
              |> React.array}
           </div>
         </div>
       | None => React.null
       }}
    </div>
    <div className=Styles.paneSpacer />
    <div>
      <CurrentRecord
        roomTrackWithMetadata
        roomRecord
        isLoggedIn={Belt.Option.isSome(user)}
      />
    </div>
  </div>;
};