type room = {
  id: string,
  connections: array(SocketMessage.connection),
  trackState: option(SocketMessage.trackState),
};

type state = {
  rooms: Js.Dict.t(room),
  currentRoomId: option(string),
};
type action =
  | AddConnection(string, SocketMessage.connection)
  | RemoveConnection(string, string)
  | UpdateRoom(room)
  | UpdateTrackState(string, SocketMessage.trackState)
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
    | UpdateRoom(room) =>
      let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
      clone->Js.Dict.set(room.id, room);
      {...state, rooms: clone};
    | UpdateTrackState(roomId, trackState) =>
      switch (Js.Dict.get(state.rooms, roomId)) {
      | Some((room: room)) =>
        let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
        clone->Js.Dict.set(roomId, {...room, trackState: Some(trackState)});
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
let updateTrackState = (roomId, trackState) =>
  api.dispatch(UpdateTrackState(roomId, trackState));
let addConnection = (roomId, connection) =>
  api.dispatch(AddConnection(roomId, connection));
let removeConnection = (roomId, connectionId) =>
  api.dispatch(RemoveConnection(roomId, connectionId));