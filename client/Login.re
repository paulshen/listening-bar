let redirectUrl = Constants.clientUrl ++ "/oauth/callback";
let spotifyClientId = "bb88a3fa1c6e4848b265da3319dabcd3";
let spotifyAuthorizeUrl =
  "https://accounts.spotify.com/authorize?client_id="
  ++ spotifyClientId
  ++ "&response_type=code&redirect_uri="
  ++ redirectUrl
  ++ "&scope=user-modify-playback-state user-read-playback-state user-read-currently-playing";

[@react.component]
let make = () => {
  React.useEffect0(() => {
    open Webapi.Dom;
    window->Window.location->Location.setHref(spotifyAuthorizeUrl);
    None;
  });
  React.null;
};