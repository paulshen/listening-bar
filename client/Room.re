module LoggedInRoom = {
  [@react.component]
  let make = (~roomId: string, ~user: UserStore.user) => {
    let room = RoomStore.useRoom(roomId);

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