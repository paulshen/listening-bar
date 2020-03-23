type room = SocketMessage.clientRoom;

type state = Js.Dict.t(room);
type action =
  | AddUser(string, string)
  | UpdateRoom(room);

let api =
  Restorative.createStore(Js.Dict.empty(), (state, action) => {
    switch (action) {
    | AddUser(roomId, userId) =>
      switch (Js.Dict.get(state, roomId)) {
      | Some((room: room)) =>
        if (room.userIds |> Js.Array.includes(userId)) {
          state;
        } else {
          let clone = Js.Dict.fromArray(Js.Dict.entries(state));
          clone->Js.Dict.set(
            roomId,
            {...room, userIds: room.userIds |> Js.Array.concat([|userId|])},
          );
          clone;
        }
      | None => state
      }

    | UpdateRoom(room) =>
      let clone = Js.Dict.fromArray(Js.Dict.entries(state));
      clone->Js.Dict.set(room.id, room);
      clone;
    }
  });

let useRoom = roomId =>
  api.useStoreWithSelector(state => Js.Dict.get(state, roomId), ());

let updateRoom = (room: room) => api.dispatch(UpdateRoom(room));
let addUser = (roomId, userId) => api.dispatch(AddUser(roomId, userId));