open Belt;

let redirectUrl = Constants.clientUrl ++ "/oauth/callback";
let spotifyClientId = "bb88a3fa1c6e4848b265da3319dabcd3";
let spotifyAuthorizeUrl =
  "https://accounts.spotify.com/authorize?client_id="
  ++ spotifyClientId
  ++ "&response_type=code&redirect_uri="
  ++ redirectUrl
  ++ "&scope=user-modify-playback-state user-read-playback-state";

let clientLogin = () => {
  let url = ReasonReactRouter.dangerouslyGetInitialUrl();
  Webapi.Dom.(
    window
    ->Window.location
    ->Location.setHref(
        spotifyAuthorizeUrl
        ++ "&state="
        ++ Js.Array.joinWith("/", Belt.List.toArray(url.path)),
      )
  );
};

let login = code => {
  let%Repromise.Js response =
    Fetch.fetchWithInit(
      Constants.serverUrl ++ "/login",
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
  let userId =
    jsonDict
    ->Js.Dict.unsafeGet("userId")
    ->Js.Json.decodeString
    ->Option.getExn;
  UserStore.login(sessionId, accessToken, userId);
  Promise.resolved(accessToken);
};

let logout = () => {
  let sessionId = UserStore.getSessionId();
  let _ =
    switch (sessionId) {
    | Some(sessionId) =>
      let%Repromise.Js response =
        Fetch.fetchWithInit(
          Constants.serverUrl ++ "/logout",
          Fetch.RequestInit.make(
            ~method_=Post,
            ~headers=Fetch.HeadersInit.make({"Authorization": sessionId}),
            ~mode=CORS,
            (),
          ),
        );
      Promise.resolved();
    | None => Promise.resolved()
    };
  Dom.Storage.(localStorage |> removeItem("sessionId"));
  ClientSocket.logout();
  UserStore.logout();
};

let fetchUser = () => {
  switch (UserStore.getSessionId()) {
  | Some(sessionId) =>
    let%Repromise.Js response =
      Fetch.fetchWithInit(
        Constants.serverUrl ++ "/user",
        Fetch.RequestInit.make(
          ~method_=Get,
          ~headers=Fetch.HeadersInit.make({"Authorization": sessionId}),
          ~mode=CORS,
          (),
        ),
      );
    switch (response) {
    | Ok(response) =>
      let status = Fetch.Response.status(response);
      if (status == 200) {
        let%Repromise.Js json = Fetch.Response.json(response);
        let json = Result.getExn(json);
        open Json.Decode;
        let userId = json |> field("userId", string);
        let accessToken = json |> field("accessToken", string);
        UserStore.updateUser({id: userId, accessToken});
        Promise.resolved();
      } else {
        UserStore.fetchFail();
        Promise.resolved();
      };
    | Error(_) =>
      UserStore.fetchFail();
      Promise.resolved();
    };
  | None =>
    UserStore.fetchFail();
    Promise.resolved();
  };
};