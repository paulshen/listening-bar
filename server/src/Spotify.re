open Belt;

%bs.raw
"var fetch = require('node-fetch').default;";

Dotenv.config();

[@bs.val] external encodeURIComponent: string => string = "encodeURIComponent";
[@bs.module "uuid"] external uuidV4: unit => string = "v4";

let getUserId = accessToken => {
  let%Repromise.Js response =
    Fetch.fetchWithInit(
      "https://api.spotify.com/v1/me",
      Fetch.RequestInit.make(
        ~method_=Get,
        ~headers=
          Fetch.HeadersInit.make({"Authorization": "Bearer " ++ accessToken}),
        (),
      ),
    );

  let%Repromise.Js json = Fetch.Response.json(Result.getExn(response));
  json
  ->Result.getExn
  ->Js.Json.decodeObject
  ->Option.getExn
  ->Js.Dict.unsafeGet("id")
  ->Js.Json.decodeString
  ->Option.getExn
  ->Promise.resolved;
};

let getToken = code => {
  let formData = [|
    (
      "client_id",
      Node.Process.process##env->Js.Dict.unsafeGet("SPOTIFY_CLIENT_ID"),
    ),
    (
      "client_secret",
      Node.Process.process##env->Js.Dict.unsafeGet("SPOTIFY_CLIENT_SECRET"),
    ),
    ("code", code),
    ("grant_type", "authorization_code"),
    ("redirect_uri", "http://localhost:8000/oauth/callback"),
  |];
  let encodedFormData =
    formData
    |> Js.Array.map(((key, value)) =>
         encodeURIComponent(key) ++ "=" ++ encodeURIComponent(value)
       )
    |> Js.Array.joinWith("&");
  let%Repromise.Js response =
    Fetch.fetchWithInit(
      "https://accounts.spotify.com/api/token",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=Fetch.BodyInit.make(encodedFormData),
        ~headers=
          Fetch.HeadersInit.make({
            "Content-Type": "application/x-www-form-urlencoded",
          }),
        (),
      ),
    );
  let%Repromise.Js json = Fetch.Response.json(Result.getExn(response));
  let jsonDict = Js.Json.decodeObject(Result.getExn(json))->Option.getExn;

  let accessToken =
    jsonDict
    ->Js.Dict.unsafeGet("access_token")
    ->Js.Json.decodeString
    ->Option.getExn;
  let refreshToken =
    jsonDict
    ->Js.Dict.unsafeGet("refresh_token")
    ->Js.Json.decodeString
    ->Option.getExn;

  let%Repromise userId = getUserId(accessToken);
  Persist.addUser({id: userId, accessToken, refreshToken});
  let sessionId = uuidV4();
  Persist.addSession({id: sessionId, userId});

  Promise.resolved(sessionId);
};