module Styles = {
  open Css;
  let root =
    style([
      display(flexBox),
      fontSize(px(14)),
      justifyContent(center),
      paddingTop(px(64)),
      paddingBottom(px(64)),
      position(relative),
      media(
        "(max-width: 720px)",
        [
          display(block),
          width(px(320)),
          marginLeft(auto),
          marginRight(auto),
          paddingTop(px(32)),
        ],
      ),
    ]);
  let leftColumn = style([paddingBottom(px(64))]);
  let paneSpacer =
    style([
      width(px(64)),
      media("(min-width: 1000px)", [width(px(128))]),
      media("(min-width: 1200px)", [width(px(64))]),
      media("(min-width: 1400px)", [width(px(128))]),
    ]);
  let recordPlayer = style([marginBottom(px(32))]);
  let smallLabel =
    style([
      fontSize(px(10)),
      letterSpacing(pxFloat(2.)),
      textTransform(uppercase),
    ]);
  let roomName = style([fontSize(px(24)), marginBottom(px(32))]);
  let syncStatus = style([marginBottom(px(12))]);
  let spotifyButton =
    style([
      backgroundColor(hex("1DB954E0")),
      color(hex("f0f0f0")),
      hover([backgroundColor(hex("1DB954"))]),
      marginBottom(px(32)),
    ]);
  let syncRow =
    style([display(flexBox), alignItems(center), marginBottom(px(32))]);
  let smallLink =
    style([
      fontSize(px(12)),
      textTransform(uppercase),
      opacity(0.5),
      hover([opacity(1.)]),
      marginLeft(px(16)),
    ]);
  let controlToggle = style([flexGrow(1.), textAlign(`right)]);
  let foreverRoomLabel =
    style([
      borderTop(px(1), solid, rgba(253, 254, 195, 0.2)),
      marginTop(px(-24)),
      marginBottom(px(32)),
      paddingTop(px(8)),
    ]);
  let connectedLabel =
    style([
      marginBottom(px(4)),
      letterSpacing(pxFloat(1.)),
      textTransform(uppercase),
    ]);
  let connectedUsername =
    style([display(inlineBlock), marginRight(px(8))]);
  let aboutLink = style([]);
  let about =
    style([
      backgroundColor(rgba(62, 49, 43, 0.98)),
      position(absolute),
      top(zero),
      left(zero),
      right(zero),
      minHeight(pct(100.)),
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
            && Belt.Option.map(localContext, context => context.type_)
            == Some("album")
            && Belt.Option.map(localContext, context => context.id)
            == Some(track.albumId)
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
    let startFrom =
      style([
        fontSize(px(10)),
        letterSpacing(pxFloat(2.)),
        marginTop(px(3)),
        opacity(0.5),
        textTransform(uppercase),
      ]);
  };

  [@react.component]
  let make = () => {
    let {currentTrack, playerState}: SpotifyStore.state =
      SpotifyStore.useState();
    let currentTrack =
      switch (currentTrack, playerState) {
      | (Some((track, _)), Playing(startTimestamp)) =>
        if (startTimestamp +. track.durationMs > Js.Date.now()) {
          currentTrack;
        } else {
          None;
        }
      | _ => None
      };
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
          <div className=Styles.startFrom>
            {React.string("Starting From")}
          </div>
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
        borderTop(px(1), solid, rgba(253, 254, 195, 0.2)),
        marginTop(px(-24)),
        marginBottom(px(32)),
        paddingTop(px(8)),
      ]);
    let controlButtonsRow =
      style([display(flexBox), alignItems(center), marginBottom(px(12))]);
    let removeRecord =
      style([
        fontSize(px(12)),
        textTransform(uppercase),
        opacity(0.5),
        hover([opacity(1.)]),
        marginLeft(px(16)),
      ]);
    let metaText = style([fontSize(px(12)), opacity(0.5)]);
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
      <div className=Styles.controlButtonsRow>
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
          {React.string(hasRecordPlaying ? "Change Album" : "Put on Album")}
        </Button>
        {hasRecordPlaying
           ? <a
               href="#"
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 ClientSocket.removeRecord(roomId) |> ignore;
               }}
               className=Styles.removeRecord>
               {React.string("Remove Album")}
             </a>
           : React.null}
      </div>
      <div className=Styles.metaText>
        {React.string(
           "This will use your currently playing track on Spotify.",
         )}
      </div>
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
let make = (~roomId: string, ~showAbout) => {
  let hasFetchedUser = UserStore.useHasFetched();
  let user = UserStore.useUser();
  let room = RoomStore.useRoom(Js.String.toLowerCase(roomId));
  let (isSyncing, setIsSyncing) = React.useState(() => false);
  let isForeverRoom =
    Constants.foreverRoomIds
    |> Js.Array.includes(Js.String.toLowerCase(roomId));

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
    Belt.Option.flatMap(
      roomRecord, ((_userId, _albumId, tracks, startTimestamp)) => {
      Belt.Option.map(
        SocketMessage.getCurrentTrack(
          tracks
          |> Js.Array.map((track: SocketMessage.roomTrack) =>
               track.durationMs
             ),
          startTimestamp,
          isForeverRoom,
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
  // Handle a forever album repeating from the first track (roomRecord doesn't change)
  React.useEffect1(
    () => {
      switch (isForeverRoom, isSyncing, roomTrackWithMetadata) {
      | (true, true, Some((_, 0, _))) =>
        syncSpotify(
          ~roomTrackWithMetadata=Belt.Option.getExn(roomTrackWithMetadata),
          ~force=true,
        )
        |> ignore
      | _ => ()
      };
      None;
    },
    [|roomTrackWithMetadata|],
  );
  React.useEffect1(
    () =>
      if (isSyncing) {
        SpotifyClient.turnOffShuffle() |> ignore;
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

  let (showControls, setShowControls) = React.useState(() => false);
  let amOnlyOne =
    switch (room) {
    | Some(room) => Js.Array.length(room.connections) == 1
    | None => false
    };
  if (amOnlyOne && !showControls && !isForeverRoom) {
    setShowControls(_ => true);
  };

  <>
    <HeaderBar
      roomId
      nav={
        showAbout
          ? <a
              href={j|/$roomId|j}
              onClick={e => {
                ReactEvent.Mouse.preventDefault(e);
                ReasonReactRouter.replace({j|/$roomId|j});
              }}
              className=Styles.aboutLink>
              {React.string("Return to room")}
            </a>
          : <a
              href={j|/$roomId#about|j}
              onClick={e => {
                ReactEvent.Mouse.preventDefault(e);
                ReasonReactRouter.replace({j|/$roomId#about|j});
              }}
              className=Styles.aboutLink>
              {React.string("About")}
            </a>
      }
    />
    <div className=Styles.root>
      <div className=Styles.leftColumn>
        <RecordPlayer
          userId={
            switch (isForeverRoom, roomRecord, roomTrackWithMetadata) {
            | (false, Some((userId, _, _, _)), Some(_)) => Some(userId)
            | _ => None
            }
          }
          startTimestamp={
            switch (roomRecord, roomTrackWithMetadata) {
            | (Some((_, _, _, startTimestamp)), Some(_)) =>
              Some(startTimestamp)
            | _ => None
            }
          }
          totalDuration={Belt.Option.map(
            roomRecord,
            ((_, _, tracks, _)) => {
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
        <div className=Styles.smallLabel> {React.string("Room")} </div>
        <div className=Styles.roomName> {React.string(roomId)} </div>
        {switch (hasFetchedUser, user) {
         | (false, _) => React.null
         | (true, Some(_user)) =>
           <div>
             <div className=Styles.syncStatus>
               {isSyncing
                  ? <div>
                      {React.string(
                         "If you leave this page, Spotify will stop syncing.",
                       )}
                    </div>
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
                    className=Styles.smallLink>
                    {React.string(isSyncing ? "Resync" : "One-time Sync")}
                  </a>
                | None => React.null
                }}
               {!isForeverRoom && !amOnlyOne
                  ? <div className=Styles.controlToggle>
                      <a
                        href="#"
                        onClick={e => {
                          ReactEvent.Mouse.preventDefault(e);
                          setShowControls(show => !show);
                        }}
                        className=Styles.smallLink>
                        {React.string(
                           showControls ? "Hide Controls" : "Show Controls",
                         )}
                      </a>
                    </div>
                  : React.null}
             </div>
             {isForeverRoom
                ? <div className=Styles.foreverRoomLabel>
                    {React.string(
                       "This is a special room looping the same album.",
                     )}
                  </div>
                : showControls
                    ? <ControlButtons
                        roomId
                        hasRecordPlaying={Belt.Option.isSome(
                          roomTrackWithMetadata,
                        )}
                      />
                    : React.null}
           </div>
         | (true, None) =>
           <div>
             <div className=Styles.syncStatus>
               {React.string("Login to sync your Spotify and change records.")}
             </div>
             <Button
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 API.clientLogin();
               }}
               className=Styles.spotifyButton>
               {React.string("Login with Spotify")}
             </Button>
           </div>
         }}
        {switch (room) {
         | Some(room) =>
           <div>
             <div className=Styles.connectedLabel>
               {React.string(
                  string_of_int(Js.Array.length(room.connections))
                  ++ " in room",
                )}
             </div>
             <div>
               {room.connections
                |> Js.Array.map((connection: SocketMessage.connection) =>
                     <div
                       className=Styles.connectedUsername key={connection.id}>
                       {React.string(
                          switch (connection.userId) {
                          | "" => "unknown"
                          | userId => userId
                          },
                        )}
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
          onTrackFinish={() => {
            // Hack rare callback here
            Js.Global.setTimeout(() => {forceUpdate(x => x + 1)}, 0)
            |> ignore
          }}
          isLoggedIn={Belt.Option.isSome(user)}
        />
      </div>
      {showAbout
         ? <div className=Styles.about> <About roomId /> </div> : React.null}
    </div>
  </>;
};