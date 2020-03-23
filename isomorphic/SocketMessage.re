type clientRoom = {
  id: string,
  userIds: array(string),
};

type clientToServer =
  | JoinRoom(string, string);
type serverToClient =
  | NewUser(string, string)
  | Connected(clientRoom);