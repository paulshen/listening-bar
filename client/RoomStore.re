type room = {
  id: string,
  userIds: array(string),
  trackState: option(SocketMessage.trackState),
};

type state = {
  rooms: Js.Dict.t(room),
  currentRoomId: option(string),
};
type action =
  | AddUser(string, string)
  | UpdateRoom(room)
  | UpdateTrackState(string, SocketMessage.trackState)
  | UpdateRoomId(option(string));

let api =
  Restorative.createStore(
    {rooms: Js.Dict.empty(), currentRoomId: None}, (state, action) => {
    switch (action) {
    | AddUser(roomId, userId) =>
      switch (Js.Dict.get(state.rooms, roomId)) {
      | Some((room: room)) =>
        if (room.userIds |> Js.Array.includes(userId)) {
          state;
        } else {
          let clone = Js.Dict.fromArray(Js.Dict.entries(state.rooms));
          clone->Js.Dict.set(
            roomId,
            {...room, userIds: room.userIds |> Js.Array.concat([|userId|])},
          );
          {...state, rooms: clone};
        }
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
let addUser = (roomId, userId) => api.dispatch(AddUser(roomId, userId));