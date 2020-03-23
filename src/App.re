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
    React.null;
  | ["login"] => <div> <Login /> </div>
  };
};