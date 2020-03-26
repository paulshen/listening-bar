type room = {
  id: string,
  connections: array(SocketMessage.connection),
  record: option((string, string, array(SocketMessage.roomTrack), float)),
};

type state = {
  rooms: Js.Dict.t(room),
  currentRoomId: option(string),
};
type action =
  | AddConnection(string, SocketMessage.connection)
  | RemoveConnection(string, string)
  | LogoutConnection(string, string)
  | UpdateRoom(room)
  | StartRecord(
      string,
      string,
      string,
      array(SocketMessage.roomTrack),
      float,
    )
  | RemoveRecord(string)
  | UpdateRoomId(option(string));

let api =
  Restorative.createStore(
    {rooms: Js.Dict.empty(), currentRoomId: None}, (state, action) => {
    switch (action) {
    | AddConnection(roomId, connection) =>
      switch (Js.Dict.get(state.rooms, roomId)) {
      | Some((room: room)) =>
        let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
        clone->Js.Dict.set(
          roomId,
          {
            ...room,
            connections: room.connections |> Js.Array.concat([|connection|]),
          },
        );
        {...state, rooms: clone};
      | None => state
      }
    | RemoveConnection(roomId, connectionId) =>
      switch (Js.Dict.get(state.rooms, roomId)) {
      | Some((room: room)) =>
        let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
        clone->Js.Dict.set(
          roomId,
          {
            ...room,
            connections:
              room.connections
              |> Js.Array.filter((connection: SocketMessage.connection) =>
                   connection.id != connectionId
                 ),
          },
        );
        {...state, rooms: clone};
      | None => state
      }
    | LogoutConnection(roomId, connectionId) =>
      switch (Js.Dict.get(state.rooms, roomId)) {
      | Some((room: room)) =>
        let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
        clone->Js.Dict.set(
          roomId,
          {
            ...room,
            connections:
              room.connections
              |> Js.Array.map((connection: SocketMessage.connection) =>
                   connection.id != connectionId
                     ? connection : {id: connectionId, userId: ""}
                 ),
          },
        );
        {...state, rooms: clone};
      | None => state
      }
    | UpdateRoom(room) =>
      let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
      clone->Js.Dict.set(room.id, room);
      {...state, rooms: clone};
    | StartRecord(roomId, userId, albumId, roomTracks, startTimestamp) =>
      switch (Js.Dict.get(state.rooms, roomId)) {
      | Some((room: room)) =>
        let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
        clone->Js.Dict.set(
          roomId,
          {
            ...room,
            record: Some((userId, albumId, roomTracks, startTimestamp)),
          },
        );
        {...state, rooms: clone};
      | None => state
      }
    | RemoveRecord(roomId) =>
      switch (Js.Dict.get(state.rooms, roomId)) {
      | Some((room: room)) =>
        let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
        clone->Js.Dict.set(roomId, {...room, record: None});
        {...state, rooms: clone};
      | None => state
      }
    | UpdateRoomId(roomId) => {...state, currentRoomId: roomId}
    }
  });

let useRoom = roomId =>
  api.useStoreWithSelector(state => Js.Dict.get(state.rooms, roomId), ());
let getCurrentRoomId = () => api.getState().currentRoomId;

let updateCurrentRoomId = roomId => api.dispatch(UpdateRoomId(roomId));
let updateRoom = (room: room) => api.dispatch(UpdateRoom(room));
let startRecord = (roomId, userId, albumId, roomTracks, startTimestamp) =>
  api.dispatch(
    StartRecord(roomId, userId, albumId, roomTracks, startTimestamp),
  );
let removeRecord = roomId => api.dispatch(RemoveRecord(roomId));
let addConnection = (roomId, connection) =>
  api.dispatch(AddConnection(roomId, connection));
let removeConnection = (roomId, connectionId) =>
  api.dispatch(RemoveConnection(roomId, connectionId));
let logoutConnection = (roomId, connectionId) =>
  api.dispatch(LogoutConnection(roomId, connectionId));