module Styles = {
  open Css;
  let connectedLabel =
    style([
      letterSpacing(pxFloat(1.)),
      textTransform(uppercase),
      marginBottom(px(4)),
    ]);
  let connectedUsername =
    style([display(inlineBlock), marginRight(px(10))]);
  let anonymousToken =
    style([display(inlineBlock), marginLeft(px(4)), opacity(0.5)]);
  let anonymousTokenLink = style([]);
};

[@react.component]
let make = (~room: option(RoomStore.room), ~user: option(UserStore.user)) => {
  let shouldShowNameLink =
    ref(
      switch (user) {
      | Some(user) => user.anonymous
      | None => false
      },
    );
  switch (room) {
  | Some(room) =>
    let seenUserIds = [||];
    let connections =
      room.connections
      |> Js.Array.filter((connection: SocketMessage.connection) => {
           switch (connection.userId) {
           | "" => true
           | "anonymous" => true
           | userId =>
             if (seenUserIds |> Js.Array.includes(userId)) {
               false;
             } else {
               seenUserIds |> Js.Array.push(userId) |> ignore;
               true;
             }
           }
         });
    <div>
      <div className=Styles.connectedLabel>
        {React.string(
           string_of_int(Js.Array.length(connections)) ++ " in room",
         )}
      </div>
      <div>
        {connections
         |> Js.Array.map((connection: SocketMessage.connection) =>
              <div className=Styles.connectedUsername key={connection.id}>
                {React.string(
                   switch (connection.userId) {
                   | "" => "unknown"
                   | userId => userId
                   },
                 )}
                {if (connection.userId == "anonymous" && shouldShowNameLink^) {
                   shouldShowNameLink := false;
                   <span className=Styles.anonymousToken>
                     {React.string("(")}
                     <a
                       href="#"
                       onClick={e => {
                         ReactEvent.Mouse.preventDefault(e);
                         ClientSocket.setAnonymous(
                           Belt.Option.getExn(UserStore.getSessionId()),
                           room.id,
                           false,
                         );
                       }}
                       className=Styles.anonymousTokenLink>
                       {React.string("show my name")}
                     </a>
                     {React.string(")")}
                   </span>;
                 } else if (Belt.Option.map(user, user => user.id)
                            == Some(connection.userId)) {
                   <span className=Styles.anonymousToken>
                     {React.string("(")}
                     <a
                       href="#"
                       onClick={e => {
                         ReactEvent.Mouse.preventDefault(e);
                         ClientSocket.setAnonymous(
                           Belt.Option.getExn(UserStore.getSessionId()),
                           room.id,
                           true,
                         );
                       }}
                       className=Styles.anonymousTokenLink>
                       {React.string("hide name")}
                     </a>
                     {React.string(")")}
                   </span>;
                 } else {
                   React.null;
                 }}
              </div>
            )
         |> React.array}
      </div>
    </div>;
  | None => React.null
  };
};