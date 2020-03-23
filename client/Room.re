module LoggedInRoom = {
  [@react.component]
  let make = (~roomId: string, ~user: UserStore.user) => {
    <div>
      <div> {React.string(roomId)} </div>
      <div> {React.string(user.id)} </div>
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