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

  React.useEffect1(
    () => {
      if (isSyncing) {
        SpotifyClient.turnOffRepeat() |> ignore;
      };
      None;
    },
    [|isSyncing|],
  );

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
  React.useEffect2(
    () =>
      switch (roomRecord) {
      | Some((albumId, _, _)) =>
        let (_, index, startTimestamp) =
          Belt.Option.getExn(roomTrackWithMetadata);
        let positionMs = Js.Date.now() -. startTimestamp;
        if (isSyncing) {
          SpotifyClient.playAlbum(albumId, index, positionMs) |> ignore;
        };
        None;
      | None =>
        if (isSyncing) {
          SpotifyClient.pausePlayer() |> ignore;
        };
        None;
      },
    (roomRecord, isSyncing),
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
      />
      <div> {React.string(roomId)} </div>
      {switch (user) {
       | Some(user) =>
         <div>
           <div> {React.string(user.id)} </div>
           <div>
             <button onClick={_ => setIsSyncing(sync => !sync)}>
               {React.string(isSyncing ? "Stop Sync" : "Start Sync")}
             </button>
             <button onClick={_ => {SpotifyStore.fetchIfNeeded() |> ignore}}>
               {React.string("Preview Current Track")}
             </button>
           </div>
           <div>
             <button
               onClick={_ => {
                 {
                   let%Repromise currentTrack = SpotifyStore.fetchIfNeeded();
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
               }}>
               {React.string("Put on Record")}
             </button>
             <button
               onClick={_ => {ClientSocket.removeRecord(roomId) |> ignore}}>
               {React.string("Remove Record")}
             </button>
           </div>
           <div> <LocalPlayerState /> </div>
         </div>
       | None =>
         <div> <Link path="/login"> {React.string("Login")} </Link> </div>
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