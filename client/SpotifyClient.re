open Belt;

type device = {
  id: string,
  name: string,
};

let rec getCurrentTrack = () => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let%Repromise.Js response =
      Fetch.fetchWithInit(
        "https://api.spotify.com/v1/me/player",
        Fetch.RequestInit.make(
          ~method_=Get,
          ~headers=
            Fetch.HeadersInit.make({
              "Authorization": "Bearer " ++ accessToken,
            }),
          (),
        ),
      );
    let response = Result.getExn(response);
    switch (Fetch.Response.status(response)) {
    | 401 =>
      let%Repromise _ = API.fetchUser();
      getCurrentTrack();
    | 204 => Promise.resolved(None)
    | 200 =>
      let%Repromise.Js json = Fetch.Response.json(response);
      let item =
        Result.getExn(json)
        ->Js.Json.decodeObject
        ->Option.getExn
        ->Js.Dict.unsafeGet("item");
      let track: SpotifyTypes.track =
        Json.Decode.{
          id: item |> field("id", string),
          name: item |> field("name", string),
          trackNumber: item |> field("track_number", int),
          durationMs: item |> field("duration_ms", Json.Decode.float),
          uri: item |> field("uri", string),
          album:
            item
            |> field("album", (json) =>
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
          artists:
            item
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
      let context: option(SpotifyTypes.context) =
        (
          Result.getExn(json)
          ->Js.Json.decodeObject
          ->Option.getExn
          ->Js.Dict.unsafeGet("context")
          |> Json.Decode.(
               nullable((json) =>
                 (
                   {
                     type_: json |> field("type", string),
                     id: {
                       let href = json |> field("href", string);
                       href
                       |> Js.String.substringToEnd(
                            ~from=Js.String.lastIndexOf("/", href) + 1,
                          );
                     },
                   }: SpotifyTypes.context
                 )
               )
             )
        )
        ->Js.Null.toOption;
      let isPlaying =
        Result.getExn(json) |> Json.Decode.(field("is_playing", bool));
      let progressMs =
        Result.getExn(json)
        |> Json.Decode.(field("progress_ms", Json.Decode.float));
      let startTimestamp = Js.Date.now() -. progressMs;
      Promise.resolved(Some((track, context, isPlaying, startTimestamp)));
    | _ =>
      Js.log(response);
      Promise.resolved(None);
    };
  | None => Promise.resolved(None)
  };
};

let getAvailableDevices = () => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let%Repromise.Js response =
      Fetch.fetchWithInit(
        "https://api.spotify.com/v1/me/player/devices",
        Fetch.RequestInit.make(
          ~method_=Get,
          ~headers=
            Fetch.HeadersInit.make({
              "Authorization": "Bearer " ++ accessToken,
            }),
          (),
        ),
      );
    let response = Result.getExn(response);
    switch (Fetch.Response.status(response)) {
    | 200 =>
      let%Repromise.JsExn json = Fetch.Response.json(response);
      open Json.Decode;
      let devices =
        json
        |> field(
             "devices",
             array((json) =>
               (
                 {
                   id: json |> field("id", string),
                   name: json |> field("name", string),
                 }: device
               )
             ),
           );
      Promise.resolved(Ok(devices));
    | _ => Promise.resolved(Error())
    };
  | None => Promise.resolved(Error())
  };
};

let turnOffShuffle = () => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let%Repromise.Js _ =
      Fetch.fetchWithInit(
        "https://api.spotify.com/v1/me/player/shuffle?state=false",
        Fetch.RequestInit.make(
          ~method_=Put,
          ~headers=
            Fetch.HeadersInit.make({
              "Authorization": "Bearer " ++ accessToken,
            }),
          (),
        ),
      );
    Promise.resolved();
  | None => Promise.resolved()
  };
};

let turnOffRepeat = () => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let%Repromise.Js _ =
      Fetch.fetchWithInit(
        "https://api.spotify.com/v1/me/player/repeat?state=off",
        Fetch.RequestInit.make(
          ~method_=Put,
          ~headers=
            Fetch.HeadersInit.make({
              "Authorization": "Bearer " ++ accessToken,
            }),
          (),
        ),
      );
    Promise.resolved();
  | None => Promise.resolved()
  };
};

type playError =
  | SpotifyError(int, string)
  | SelectDevice(array(device))
  | NoAvailableDevice
  | NoAccessToken;
let rec playAlbum = (albumId, offset, positionMs, ~deviceId) => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let url = "https://api.spotify.com/v1/me/player/play";
    let url =
      switch (deviceId) {
      | Some(deviceId) => url ++ "?device_id=" ++ deviceId
      | None => url
      };
    let%Repromise.Js response =
      Fetch.fetchWithInit(
        url,
        Fetch.RequestInit.make(
          ~method_=Put,
          ~body=
            Fetch.BodyInit.make(
              Js.Json.stringify(
                Js.Json.object_(
                  Js.Dict.fromArray([|
                    (
                      "context_uri",
                      Js.Json.string({j|spotify:album:$albumId|j}),
                    ),
                    (
                      "offset",
                      Js.Json.object_(
                        Js.Dict.fromArray([|
                          (
                            "position",
                            Js.Json.number(float_of_int(offset)),
                          ),
                        |]),
                      ),
                    ),
                    ("position_ms", Js.Json.number(positionMs)),
                  |]),
                ),
              ),
            ),
          ~headers=
            Fetch.HeadersInit.make({
              "Authorization": "Bearer " ++ accessToken,
              "Content-Type": "application/json",
            }),
          (),
        ),
      );
    let response = Result.getExn(response);
    switch (Fetch.Response.status(response)) {
    | 204 =>
      turnOffShuffle() |> ignore;
      turnOffRepeat() |> ignore;
      Promise.resolved(Ok());
    | 404 =>
      let%Repromise devicesResult = getAvailableDevices();
      switch (devicesResult) {
      | Ok(devices) =>
        switch (Js.Array.length(devices)) {
        | 0 => Promise.resolved(Error(NoAvailableDevice))
        | 1 =>
          playAlbum(
            albumId,
            offset,
            positionMs,
            ~deviceId=Some(Option.getExn(devices[0]).id),
          )
        | _ => Promise.resolved(Error(SelectDevice(devices)))
        }
      | Error () => Promise.resolved(Error(NoAvailableDevice))
      };
    | statusCode =>
      let%Repromise.JsExn json = Fetch.Response.json(response);
      open Json.Decode;
      let message =
        json |> field("error", json => json |> field("message", string));
      if (statusCode == 401 && message == "The access token expired") {
        // Try to refresh token
        let%Repromise _ = API.fetchUser();
        playAlbum(albumId, offset, positionMs, ~deviceId=None);
      } else {
        Promise.resolved(Error(SpotifyError(statusCode, message)));
      };
    };
  | None => Promise.resolved(Error(NoAccessToken))
  };
};

let pausePlayer = () => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let%Repromise.Js _ =
      Fetch.fetchWithInit(
        "https://api.spotify.com/v1/me/player/pause",
        Fetch.RequestInit.make(
          ~method_=Put,
          ~headers=
            Fetch.HeadersInit.make({
              "Authorization": "Bearer " ++ accessToken,
            }),
          (),
        ),
      );
    Promise.resolved();
  | None => Promise.resolved()
  };
};