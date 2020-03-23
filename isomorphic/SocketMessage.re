type clientToServer =
  | JoinRoom(string, string);
type serverToClient =
  | NewUser(string, string)
  | Connected(string, array(string));