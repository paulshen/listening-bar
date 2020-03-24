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
  name: string,
  images: array(albumImage),
};
type track = {
  id: string,
  name: string,
  uri: string,
  durationMs: float,
  album,
  artists: array(artist),
};

type context = {
  type_: string,
  id: string,
};