let invalidCharacters = [%re "/[^\\w-]/g"];

let sanitizeRoomId = roomId => {
  roomId |> Js.String.replaceByRe(invalidCharacters, "");
};