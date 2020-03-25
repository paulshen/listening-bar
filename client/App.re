[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  React.useEffect1(
    () => {
      switch (url.path) {
      | ["room", _] => API.fetchUser() |> ignore
      | ["oauth", "callback"] =>
        let code =
          url.search
          |> Js.String.split("&")
          |> Js.Array.map(component => Js.String.split("=", component))
          |> Js.Array.find(component => component[0] == "code")
          |> Belt.Option.getExn
          |> (x => x[1]);
        {
          let%Repromise accessToken = API.login(code);
          Promise.resolved();
        }
        |> ignore;
        ReasonReactRouter.push("/");
      | _ => ()
      };
      None;
    },
    [|url|],
  );
  <div>
    {switch (url.path) {
     | ["login"] => <div> <Login /> </div>
     | ["room", roomId] => <Room roomId />
     | _ => React.null
     }}
    <ReactAtmosphere.LayerContainer />
  </div>;
};