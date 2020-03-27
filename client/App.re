[@bs.val] external decodeURIComponent: string => string = "decodeURIComponent";

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  React.useEffect1(
    () => {
      switch (url.path) {
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
      | _ => API.fetchUser() |> ignore
      };
      None;
    },
    [|url|],
  );
  <div>
    {switch (url.path) {
     | [roomId] => <Room roomId showAbout={url.hash == "about"} key=roomId />
     | [] => <IndexPage />
     | _ => React.null
     }}
    <ReactAtmosphere.LayerContainer />
  </div>;
};