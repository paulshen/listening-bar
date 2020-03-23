open Belt;

let login = code => {
  let%Repromise.Js response =
    Fetch.fetchWithInit(
      "http://localhost:3000/login",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Js.Json.object_(
                Js.Dict.fromArray([|("code", Js.Json.string(code))|]),
              ),
            ),
          ),
        ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
        ~mode=CORS,
        (),
      ),
    );
  let%Repromise.Js json = Fetch.Response.json(Result.getExn(response));
  let sessionId =
    json
    ->Result.getExn
    ->Js.Json.decodeObject
    ->Option.getExn
    ->Js.Dict.unsafeGet("sessionId")
    ->Js.Json.decodeString
    ->Option.getExn;
  Dom.Storage.(localStorage |> setItem("sessionId", sessionId));
  Promise.resolved();
};