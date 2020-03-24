type artist = {
  id: string,
  name: string,
};
type albumImage = {
  url: string,
  width: int,
  height: int,
};
type album = {
  id: string,
  images: array(albumImage),
  name: string,
};
type track = {
  id: string,
  name: string,
  uri: string,
  durationMs: float,
  album,
  artists: array(artist),
};