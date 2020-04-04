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
  let mobileNotice =
    style([
      marginBottom(px(32)),
      fontSize(px(10)),
      letterSpacing(pxFloat(2.)),
      lineHeight(px(14)),
      textTransform(uppercase),
      width(px(320)),
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
  let listenAlongButtonEmpty = style([opacity(0.5)]);
  let smallLink =
    style([
      fontSize(px(12)),
      textTransform(uppercase),
      opacity(0.5),
      hover([opacity(1.)]),
      marginLeft(px(16)),
    ]);
  let controlToggle = style([flexGrow(1.), textAlign(`right)]);
  let spotifyPlayError =
    style([
      marginTop(px(-24)),
      marginBottom(px(32)),
      paddingTop(px(4)),
    ]);
  let spotifyPlayErrorLabel =
    style([color(hex("ff7c7c")), fontSize(px(12))]);
  let foreverRoomLabel =
    style([
      borderTop(px(1), solid, rgba(253, 254, 195, 0.2)),
      marginTop(px(-24)),
      marginBottom(px(32)),
      paddingTop(px(8)),
    ]);
  let learnMoreLink = style([color(hex("FDFEC3c0"))]);
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
    (
      ~activeTrackWithMetadata: (SocketMessage.roomTrack, int, float),
      ~onError,
      ~force,
    ) => {
  let (track, index, startTimestamp) = activeTrackWithMetadata;
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
    let%Repromise response =
      SpotifyClient.playAlbum(
        track.albumId,
        index,
        positionMs,
        ~deviceId=SpotifyStore.getDeviceId(),
      );
    switch (response) {
    | Ok () => SpotifyStore.updatePlayError(None)
    | Error(playError) =>
      onError(playError);
      SpotifyStore.updatePlayError(Some(playError));
    };
    Promise.resolved(response);
  } else {
    Promise.resolved(Ok());
  };
};

module SpotifyChooseDevice = {
  module Styles = {
    open Css;
    let label =
      style([
        color(hex("ff7c7c")),
        fontSize(px(12)),
        marginBottom(px(2)),
      ]);
    let selected =
      style([fontWeight(`num(600)), textDecoration(underline)]);
  };

  [@react.component]
  let make = (~availableDevices) => {
    let deviceId = SpotifyStore.useDeviceId();
    let onDeviceClick = (e, deviceId) => {
      SpotifyStore.setDeviceId(deviceId);
      ReactEvent.Mouse.preventDefault(e);
    };
    <div>
      <div className=Styles.label>
        {React.string("Select an available device and try again")}
      </div>
      {availableDevices
       |> Js.Array.map((device: SpotifyClient.device) =>
            <div key={device.id}>
              <a
                href="#"
                onClick={e => onDeviceClick(e, device.id)}
                className={Cn.ifTrue(
                  Styles.selected,
                  deviceId == Some(device.id),
                )}>
                {React.string(device.name)}
              </a>
            </div>
          )
       |> React.array}
    </div>;
  };
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
      SpotifyStore.fetchIfNeeded(~bufferMs=5000) |> ignore;
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

[@bs.scope "navigator"] [@bs.val] external userAgent: string = "userAgent";
let mobileRegex = [%re "/iPhone|iPod|Android/i"];
let isMobile = userAgent |> Js.Re.test_(mobileRegex);

[@react.component]
let make = (~roomId: string, ~showAbout) => {
  let hasFetchedUser = UserStore.useHasFetched();
  let user = UserStore.useUser();
  let room = RoomStore.useRoom(Js.String.toLowerCase(roomId));
  let (isSyncing, setIsSyncing) = React.useState(() => false);
  let isLoggedIn = Belt.Option.isSome(user);
  let isForeverRoom =
    Constants.foreverRoomIds
    |> Js.Array.includes(Js.String.toLowerCase(roomId));

  if (!isLoggedIn && isSyncing) {
    setIsSyncing(_ => false);
  };

  React.useEffect0(() => {
    Webapi.Dom.(document->Document.documentElement->Element.setScrollTop(0.));
    ClientSocket.connectToRoom(roomId, UserStore.getSessionId());
    None;
  });

  let (_, forceUpdate) = React.useState(() => 1);
  let roomRecord = Belt.Option.flatMap(room, room => room.record);
  let activeTrackWithMetadata =
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
  let activeTrackWithMetadataRef = React.useRef(activeTrackWithMetadata);
  React.useEffect1(
    () => {
      React.Ref.setCurrent(
        activeTrackWithMetadataRef,
        activeTrackWithMetadata,
      );
      None;
    },
    [|activeTrackWithMetadata|],
  );
  let timeLoadedRoomRef = React.useRef(None);
  if (Belt.Option.isNone(React.Ref.current(timeLoadedRoomRef))) {
    React.Ref.setCurrent(timeLoadedRoomRef, Some(Js.Date.now()));
  };
  let hasActiveTrack = Belt.Option.isSome(activeTrackWithMetadata);
  React.useEffect2(
    () => {
      if (isLoggedIn
          && !isSyncing
          && hasActiveTrack
          && Js.Date.now()
          -. Belt.Option.getExn(React.Ref.current(timeLoadedRoomRef)) < 3000.) {
        setIsSyncing(_ => true);
      };
      None;
    },
    (isLoggedIn, hasActiveTrack),
  );
  let onSyncError = _ => {
    setIsSyncing(_ => false);
  };
  React.useEffect2(
    () => {
      switch (roomRecord) {
      | Some(_) =>
        if (isSyncing) {
          syncSpotify(
            ~activeTrackWithMetadata=
              Belt.Option.getExn(activeTrackWithMetadata),
            ~onError=onSyncError,
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
  let roomTrackOffset =
    Belt.Option.map(activeTrackWithMetadata, ((_, index, _)) => index);
  React.useEffect1(
    () => {
      switch (isForeverRoom, isSyncing, roomTrackOffset) {
      | (true, true, Some(0)) =>
        syncSpotify(
          ~activeTrackWithMetadata=
            Belt.Option.getExn(activeTrackWithMetadata),
          ~onError=onSyncError,
          ~force=true,
        )
        |> ignore
      | _ => ()
      };
      None;
    },
    [|roomTrackOffset|],
  );
  React.useEffect1(
    () =>
      if (isSyncing) {
        SpotifyClient.turnOffShuffle() |> ignore;
        SpotifyClient.turnOffRepeat() |> ignore;
        let interval =
          Js.Global.setInterval(
            () => {
              switch (React.Ref.current(activeTrackWithMetadataRef)) {
              | Some(activeTrackWithMetadata) =>
                syncSpotify(
                  ~activeTrackWithMetadata,
                  ~onError=onSyncError,
                  ~force=false,
                )
                |> ignore
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
    Belt.Option.map(activeTrackWithMetadata, ((track, _, _)) =>
      track.trackId
    );
  React.useEffect1(
    () =>
      switch (roomTrackId) {
      | Some(_) =>
        let (roomTrack, index, startTimestamp) =
          Belt.Option.getExn(activeTrackWithMetadata);
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

  let amOnlyOne =
    switch (room) {
    | Some(room) => Js.Array.length(room.connections) == 1
    | None => false
    };
  let (showControls, setShowControls) =
    React.useState(() =>
      amOnlyOne || Belt.Option.isNone(activeTrackWithMetadata)
    );
  React.useEffect1(
    () => {
      if (Belt.Option.isNone(activeTrackWithMetadata) && !showControls) {
        setShowControls(_ => true);
      };
      None;
    },
    [|Belt.Option.isNone(activeTrackWithMetadata)|],
  );
  if (amOnlyOne && !showControls && !isForeverRoom) {
    setShowControls(_ => true);
  };

  let playError = SpotifyStore.useError();

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
        {isMobile
           ? <div className=Styles.mobileNotice>
               {React.string(
                  "The experience is not great on mobile. Sorry! Try it in your desktop browser.",
                )}
             </div>
           : React.null}
        {let totalDuration =
           Belt.Option.map(
             roomRecord,
             ((_, _, tracks, _)) => {
               let duration = ref(0.);
               tracks
               |> Js.Array.forEach((track: SocketMessage.roomTrack) => {
                    duration := duration^ +. track.durationMs
                  });
               duration^;
             },
           );
         <RecordPlayer
           userId={
             switch (isForeverRoom, roomRecord, activeTrackWithMetadata) {
             | (false, Some((userId, _, _, _)), Some(_)) => Some(userId)
             | _ => None
             }
           }
           startTimestamp={
             switch (roomRecord, activeTrackWithMetadata) {
             | (Some((_, _, _, startTimestamp)), Some(_)) =>
               let totalDuration = Belt.Option.getExn(totalDuration);
               Some(
                 startTimestamp
                 +. (
                   isForeverRoom
                     ? float_of_int(
                         Js.Math.floor(
                           (Js.Date.now() -. startTimestamp) /. totalDuration,
                         ),
                       )
                       *. totalDuration
                     : 0.
                 ),
               );
             | _ => None
             }
           }
           totalDuration
           className=Styles.recordPlayer
         />}
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
                  : <Button
                      onClick={_ => setIsSyncing(_ => true)}
                      className={Cn.ifTrue(
                        Styles.listenAlongButtonEmpty,
                        Belt.Option.isNone(activeTrackWithMetadata),
                      )}>
                      {React.string("Listen Along")}
                    </Button>}
               {switch (activeTrackWithMetadata) {
                | Some(activeTrackWithMetadata) =>
                  <a
                    href="#"
                    onClick={e => {
                      ReactEvent.Mouse.preventDefault(e);
                      syncSpotify(
                        ~activeTrackWithMetadata,
                        ~onError=onSyncError,
                        ~force=false,
                      )
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
             {switch (playError) {
              | Some(SpotifyClient.SelectDevice(availableDevices)) =>
                <div className=Styles.spotifyPlayError>
                  <SpotifyChooseDevice availableDevices />
                </div>
              | Some(SpotifyClient.NoAvailableDevice) =>
                <div className=Styles.spotifyPlayError>
                  <div className=Styles.spotifyPlayErrorLabel>
                    {React.string("No available Spotify devices")}
                  </div>
                </div>
              | Some(SpotifyClient.SpotifyError(status, message)) =>
                <div className=Styles.spotifyPlayError>
                  <div className=Styles.spotifyPlayErrorLabel>
                    {React.string(message)}
                  </div>
                </div>
              | _ => React.null
              }}
             {isForeverRoom
                ? <div className=Styles.foreverRoomLabel>
                    {React.string(
                       "This is a special room looping the same album. ",
                     )}
                    <a
                      href={j|/$roomId#about|j}
                      onClick={e => {
                        ReactEvent.Mouse.preventDefault(e);
                        ReasonReactRouter.replace({j|/$roomId#about|j});
                      }}
                      className=Styles.learnMoreLink>
                      {React.string("Learn More")}
                    </a>
                  </div>
                : showControls
                    ? <ControlButtons
                        roomId
                        hasRecordPlaying={Belt.Option.isSome(
                          activeTrackWithMetadata,
                        )}
                      />
                    : React.null}
           </div>
         | (true, None) =>
           <div>
             <div className=Styles.syncStatus>
               {React.string("Login to listen along and change records.")}
             </div>
             <Button
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 API.clientLogin();
               }}
               className=Styles.spotifyButton>
               {React.string("Login with Spotify Premium")}
             </Button>
           </div>
         }}
        <ConnectedUsers room user />
      </div>
      <div className=Styles.paneSpacer />
      <div>
        <CurrentRecord
          activeTrackWithMetadata
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