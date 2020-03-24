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
let make = (~roomId: string) => {
  let user = UserStore.useUser();
  let room = RoomStore.useRoom(roomId);
  let (isSyncing, setIsSyncing) = React.useState(() => false);

  React.useEffect0(() => {
    ClientSocket.connectToRoom(roomId, UserStore.getSessionId());
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
    {switch (user) {
     | Some(user) =>
       <div>
         <div> {React.string(user.id)} </div>
         <button onClick={_ => setIsSyncing(sync => !sync)}>
           {React.string(isSyncing ? "Stop Sync" : "Start Sync")}
         </button>
         <button onClick={_ => publishCurrentTrack() |> ignore}>
           {React.string("Publish Current Track")}
         </button>
       </div>
     | None =>
       <div> <Link path="/login"> {React.string("Login")} </Link> </div>
     }}
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
  </div>;
};