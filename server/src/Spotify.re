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
    ("redirect_uri", Constants.clientUrl ++ "/oauth/callback"),
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
  let expiresIn =
    jsonDict
    ->Js.Dict.unsafeGet("expires_in")
    ->Js.Json.decodeNumber
    ->Option.getExn
    *. 1000.;
  let tokenExpireTime = Js.Date.now() +. expiresIn;

  let%Repromise userId = getUserId(accessToken);
  let sessionId = uuidV4();
  let%Repromise client = Database.getClient();
  let%Repromise user = Database.getUser(client, userId);
  let anonymous =
    switch (user) {
    | Some(user) => user.anonymous
    | None => false
    };
  let%Repromise _ =
    Promise.all2(
      Database.updateUser(
        client,
        {id: userId, accessToken, refreshToken, tokenExpireTime, anonymous},
      ),
      Database.addSession(client, {id: sessionId, userId}),
    );
  Database.releaseClient(client) |> ignore;
  Promise.resolved((sessionId, accessToken, userId, anonymous));
};

let refreshToken = refreshToken => {
  let formData = [|
    (
      "client_id",
      Node.Process.process##env->Js.Dict.unsafeGet("SPOTIFY_CLIENT_ID"),
    ),
    (
      "client_secret",
      Node.Process.process##env->Js.Dict.unsafeGet("SPOTIFY_CLIENT_SECRET"),
    ),
    ("refresh_token", refreshToken),
    ("grant_type", "refresh_token"),
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
  let tokenExpireTime =
    jsonDict
    ->Js.Dict.unsafeGet("expires_in")
    ->Js.Json.decodeNumber
    ->Option.getExn
    *. 1000.
    +. Js.Date.now();

  Promise.resolved((accessToken, tokenExpireTime));
};

let jsonToTrack =
    (~album: option(SpotifyTypes.album)=?, json): SpotifyTypes.track => {
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
    trackNumber: json |> field("track_number", int),
    durationMs: json |> field("duration_ms", Json.Decode.float),
    uri: json |> field("uri", string),
    album:
      switch (
        json
        |> optional(
             field("album", (json) =>
               (
                 {
                   id: json |> field("id", string),
                   name: json |> field("name", string),
                   images:
                     json
                     |> field(
                          "images",
                          array((json) =>
                            (
                              {
                                url: json |> field("url", string),
                                width: json |> field("width", int),
                                height: json |> field("height", int),
                              }: SpotifyTypes.albumImage
                            )
                          ),
                        ),
                 }: SpotifyTypes.album
               )
             ),
           )
      ) {
      | Some(album) => album
      | None => Option.getExn(album)
      },
    artists:
      json
      |> field(
           "artists",
           array((json) =>
             (
               {
                 id: json |> field("id", string),
                 name: json |> field("name", string),
               }: SpotifyTypes.artist
             )
           ),
         ),
  };
};
let jsonToAlbum = (json): SpotifyTypes.album => {
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
    images:
      json
      |> field(
           "images",
           array((json) =>
             (
               {
                 url: json |> field("url", string),
                 width: json |> field("width", int),
                 height: json |> field("height", int),
               }: SpotifyTypes.albumImage
             )
           ),
         ),
  };
};

let getTrackInfo = (~accessToken, ~trackId) => {
  let%Repromise.Js response =
    Fetch.fetchWithInit(
      "https://api.spotify.com/v1/tracks/" ++ trackId,
      Fetch.RequestInit.make(
        ~method_=Get,
        ~headers=
          Fetch.HeadersInit.make({"Authorization": "Bearer " ++ accessToken}),
        (),
      ),
    );
  let%Repromise.Js json = Fetch.Response.json(Result.getExn(response));
  let item = Result.getExn(json);
  let track: SpotifyTypes.track = jsonToTrack(item);
  Promise.resolved(track);
};

let getAlbumTracks =
    (~accessToken, ~albumId): Promise.t(array(SpotifyTypes.track)) => {
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      "https://api.spotify.com/v1/albums/" ++ albumId,
      Fetch.RequestInit.make(
        ~method_=Get,
        ~headers=
          Fetch.HeadersInit.make({"Authorization": "Bearer " ++ accessToken}),
        (),
      ),
    );
  let%Repromise.JsExn json = Fetch.Response.json(response);
  let album = jsonToAlbum(json);
  open Json.Decode;
  let tracks =
    json
    |> field("tracks", json =>
         json |> field("items", array(jsonToTrack(~album)))
       );
  Promise.resolved(tracks);
};

exception UnsupportedContextType(string);
let getContextTracks = (~accessToken, ~contextType, ~contextId) => {
  switch (contextType) {
  | "album" => getAlbumTracks(~accessToken, ~albumId=contextId)
  | _ => raise(UnsupportedContextType(contextType))
  };
};