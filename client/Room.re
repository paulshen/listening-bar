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
          track.id,
          startTimestamp,
          track.durationMs,
        );
      };
    | None => ()
    };
    Promise.resolved();
  };

  [@react.component]
  let make = (~roomId: string, ~user: UserStore.user) => {
    let room = RoomStore.useRoom(roomId);
    let {currentTrack, playerState}: SpotifyStore.state =
      SpotifyStore.useState();

    React.useEffect0(() => {
      ClientSocket.connectToRoom(
        roomId,
        UserStore.getSessionId()->Belt.Option.getExn,
      );
      None;
    });

    <div>
      <div> {React.string(roomId)} </div>
      <div> {React.string(user.id)} </div>
      <button onClick={_ => publishCurrentTrack() |> ignore}>
        {React.string("Publish Current Track")}
      </button>
      {switch (currentTrack) {
       | Some(currentTrack) =>
         <div>
           <div> <img src={currentTrack.album.images[0].url} /> </div>
           <div> {React.string(currentTrack.name)} </div>
           <div> {React.string(currentTrack.artists[0].name)} </div>
         </div>
       | None => React.null
       }}
      {switch (room) {
       | Some(room) =>
         <div>
           {room.userIds
            |> Js.Array.mapi((userId, i) =>
                 <div key={string_of_int(i)}> {React.string(userId)} </div>
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