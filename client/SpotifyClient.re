open Belt;

let rec getCurrentTrack = () => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let%Repromise.Js response =
      Fetch.fetchWithInit(
        "https://api.spotify.com/v1/me/player/currently-playing",
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
      let track: SpotifyStore.track =
        Json.Decode.{
          id: item |> field("id", string),
          name: item |> field("name", string),
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
                                }: SpotifyStore.albumImage
                              )
                            ),
                          ),
                   }: SpotifyStore.album
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
                     }: SpotifyStore.artist
                   )
                 ),
               ),
        };
      let isPlaying =
        Result.getExn(json) |> Json.Decode.(field("is_playing", bool));
      let progressMs =
        Result.getExn(json)
        |> Json.Decode.(field("progress_ms", Json.Decode.float));
      let startTimestamp = Js.Date.now() -. progressMs;
      Promise.resolved(Some((track, isPlaying, startTimestamp)));
    | _ =>
      Js.log(response);
      Promise.resolved(None);
    };
  | None => Promise.resolved(None)
  };
};

let playTrack = (trackId, positionMs) => {
  switch (Belt.Option.map(UserStore.getUser(), user => user.accessToken)) {
  | Some(accessToken) =>
    let%Repromise.Js response =
      Fetch.fetchWithInit(
        "https://api.spotify.com/v1/me/player/play",
        Fetch.RequestInit.make(
          ~method_=Put,
          ~body=
            Fetch.BodyInit.make(
              Js.Json.stringify(
                Js.Json.object_(
                  Js.Dict.fromArray([|
                    (
                      "uris",
                      Js.Json.stringArray([|{j|spotify:track:$trackId|j}|]),
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
    Promise.resolved();
  | None => Promise.resolved()
  };
};