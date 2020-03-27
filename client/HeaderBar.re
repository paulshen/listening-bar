module Styles = {
  open Css;
  let header =
    style([
      display(flexBox),
      fontSize(px(14)),
      justifyContent(spaceBetween),
      padding2(~v=px(16), ~h=px(32)),
      media(
        "(max-width: 480px)",
        [paddingLeft(px(16)), paddingRight(px(16))],
      ),
    ]);
  let title =
    style([textTransform(uppercase), letterSpacing(pxFloat(1.))]);
  let roomId = style([]);
  let headerRight = style([]);
  let logoutLink = style([marginLeft(px(24))]);
};

[@react.component]
let make = (~roomId=?, ~nav=React.null, ()) => {
  let user = UserStore.useUser();
  <div className=Styles.header>
    <div>
      <span className=Styles.title>
        {switch (roomId) {
         | Some(roomId) =>
           <a
             href={j|/$roomId#about|j}
             onClick={e => {
               ReactEvent.Mouse.preventDefault(e);
               ReasonReactRouter.replace({j|/$roomId#about|j});
             }}>
             {React.string("Listening Bar")}
           </a>
         | None => React.string("Listening Bar")
         }}
      </span>
      {switch (roomId) {
       | Some(roomId) =>
         <>
           <span className=Styles.title> {React.string(" / ")} </span>
           <span className=Styles.roomId>
             <a
               href={j|/$roomId|j}
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 ReasonReactRouter.replace({j|/$roomId|j});
               }}>
               {React.string(roomId)}
             </a>
           </span>
         </>
       | None => React.null
       }}
    </div>
    <div className=Styles.headerRight>
      nav
      {if (Belt.Option.isSome(user)) {
         <a
           href="#"
           onClick={e => {
             ReactEvent.Mouse.preventDefault(e);
             API.logout();
           }}
           className=Styles.logoutLink>
           {React.string("Logout")}
         </a>;
       } else {
         React.null;
       }}
    </div>
  </div>;
};