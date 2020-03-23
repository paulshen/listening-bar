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
  let jsonDict = json->Result.getExn->Js.Json.decodeObject->Option.getExn;
  let sessionId =
    jsonDict
    ->Js.Dict.unsafeGet("sessionId")
    ->Js.Json.decodeString
    ->Option.getExn;
  Dom.Storage.(localStorage |> setItem("sessionId", sessionId));
  let accessToken =
    jsonDict
    ->Js.Dict.unsafeGet("accessToken")
    ->Js.Json.decodeString
    ->Option.getExn;
  Dom.Storage.(localStorage |> setItem("accessToken", accessToken));
  Promise.resolved(accessToken);
};

let getUser = () => {
  switch (UserStore.getSessionId()) {
  | Some(sessionId) =>
    let%Repromise.Js response =
      Fetch.fetchWithInit(
        "http://localhost:3000/user",
        Fetch.RequestInit.make(
          ~method_=Get,
          ~headers=Fetch.HeadersInit.make({"Authorization": sessionId}),
          ~mode=CORS,
          (),
        ),
      );
    let%Repromise.Js json = Fetch.Response.json(Result.getExn(response));
    let json = Result.getExn(json);
    open Json.Decode;
    let userId = json |> field("userId", string);
    let accessToken = json |> field("accessToken", string);
    UserStore.updateUser({id: userId, accessToken});
    Promise.resolved();
  | None => Promise.resolved()
  };
};