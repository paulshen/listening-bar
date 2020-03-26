module Styles = {
  open Css;
  let header =
    style([
      display(flexBox),
      fontSize(px(14)),
      justifyContent(spaceBetween),
      padding2(~v=px(16), ~h=px(32)),
    ]);
  let title =
    style([textTransform(uppercase), letterSpacing(pxFloat(1.))]);
  let headerRight = style([]);
};

[@bs.val] external decodeURIComponent: string => string = "decodeURIComponent";

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  React.useEffect1(
    () => {
      switch (url.path) {
      | ["room", _] => API.fetchUser() |> ignore
      | ["oauth", "callback"] =>
        let components =
          url.search
          |> Js.String.split("&")
          |> Js.Array.map(component => Js.String.split("=", component));
        let code =
          components
          |> Js.Array.find(component => component[0] == "code")
          |> Belt.Option.getExn
          |> (x => x[1]);
        let state =
          components
          |> Js.Array.find(component => component[0] == "state")
          |> Belt.Option.getExn
          |> (x => x[1]);
        {
          let%Repromise accessToken = API.login(code);
          ReasonReactRouter.replace("/" ++ decodeURIComponent(state));
          Promise.resolved();
        }
        |> ignore;
      | _ => ()
      };
      None;
    },
    [|url|],
  );
  let user = UserStore.useUser();
  <div>
    <div className=Styles.header>
      <div className=Styles.title> {React.string("Listening Bar")} </div>
      <div className=Styles.headerRight>
        {if (Belt.Option.isSome(user)) {
           <a
             href="#"
             onClick={e => {
               ReactEvent.Mouse.preventDefault(e);
               API.logout();
             }}>
             {React.string("Logout")}
           </a>;
         } else {
           React.null;
         }}
      </div>
    </div>
    {switch (url.path) {
     | ["room", roomId] => <Room roomId />
     | _ => React.null
     }}
    <ReactAtmosphere.LayerContainer />
  </div>;
};