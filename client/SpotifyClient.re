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
      Promise.resolved(Some((track, isPlaying)));
    | _ =>
      Js.log(response);
      Promise.resolved(None);
    };
  | None => Promise.resolved(None)
  };
};