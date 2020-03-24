module LoggedInRoom = {
  let publishCurrentTrack = () => {
    let%Repromise result = SpotifyClient.getCurrentTrack();
    switch (result) {
    | Some((track, isPlaying, startTimestamp)) =>
      SpotifyStore.updateState(
        Some(track),
        isPlaying ? Playing(startTimestamp) : NotPlaying,
      );
      if (isPlaying) {
        ClientSocket.publishCurrentTrack(
          UserStore.getSessionId()->Belt.Option.getExn,
          track.id,
          startTimestamp,
        );
      };
    | None => ()
    };
    Promise.resolved();
  };

  [@react.component]
  let make = (~roomId: string, ~user: UserStore.user) => {
    let room = RoomStore.useRoom(roomId);
    let (isSyncing, setIsSyncing) = React.useState(() => false);

    React.useEffect0(() => {
      ClientSocket.connectToRoom(
        roomId,
        UserStore.getSessionId()->Belt.Option.getExn,
      );
      None;
    });

    let roomTrackState = Belt.Option.flatMap(room, room => room.track);
    React.useEffect2(
      () => {
        if (isSyncing) {
          switch (roomTrackState) {
          | Some((roomTrack, startTimestamp)) =>
            let positionMs = Js.Date.now() -. startTimestamp;
            SpotifyClient.playTrack(roomTrack.trackId, positionMs) |> ignore;
          | None => ()
          };
        };
        None;
      },
      (roomTrackState, isSyncing),
    );

    <div>
      <div> {React.string(roomId)} </div>
      <div> {React.string(user.id)} </div>
      <button onClick={_ => setIsSyncing(sync => !sync)}>
        {React.string(isSyncing ? "Stop Sync" : "Start Sync")}
      </button>
      <button onClick={_ => publishCurrentTrack() |> ignore}>
        {React.string("Publish Current Track")}
      </button>
      {switch (roomTrackState) {
       | Some((roomTrack, startTimestamp)) =>
         <CurrentTrack
           roomTrack
           startTimestamp
           key={roomTrack.trackId ++ Js.Float.toString(startTimestamp)}
         />
       | None => React.null
       }}
      {switch (room) {
       | Some(room) =>
         <div>
           {room.connections
            |> Js.Array.map((connection: SocketMessage.connection) =>
                 <div key={connection.id}>
                   {React.string(connection.userId)}
                 </div>
               )
            |> React.array}
         </div>
       | None => React.null
       }}
    </div>;
  };
};

[@react.component]
let make = (~roomId: string) => {
  let user = UserStore.useUser();
  switch (user) {
  | Some(user) => <LoggedInRoom roomId user />
  | None => React.null
  };
};