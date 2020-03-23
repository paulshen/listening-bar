let postToken = token => {
  Fetch.fetchWithInit(
    "http://localhost:3000/login",
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=
        Fetch.BodyInit.make(
          Js.Json.stringify(
            Js.Json.object_(
              Js.Dict.fromArray([|("token", Js.Json.string(token))|]),
            ),
          ),
        ),
      ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
      ~mode=CORS,
      (),
    ),
  )
  |> Js.Promise.then_(Fetch.Response.json);
};

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  Js.log(url);
  switch (url.path) {
  | ["oauth", "callback"] =>
    let code =
      url.search
      |> Js.String.split("&")
      |> Js.Array.map(component => Js.String.split("=", component))
      |> Js.Array.find(component => component[0] == "code")
      |> Belt.Option.getExn
      |> (x => x[1]);
    Js.log(code);
    postToken(code) |> ignore;
    React.null;
  | ["login"] => <div> <Login /> </div>
  | _ => React.null
  };
};