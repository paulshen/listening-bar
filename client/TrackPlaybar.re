let displaySeconds = seconds => {
  let minutes = seconds / 60;
  let seconds = seconds mod 60;
  Js.Int.toString(minutes)
  ++ ":"
  ++ (seconds < 10 ? "0" : "")
  ++ Js.Int.toString(seconds);
};

[@react.component]
let make = (~startTimestamp: float, ~roomTrack: SocketMessage.roomTrack) => {
  let (now, setNow) = React.useState(() => Js.Date.now());
  React.useEffect0(() => {
    let interval =
      Js.Global.setInterval(() => {setNow(_ => Js.Date.now())}, 1000);
    Some(() => {Js.Global.clearInterval(interval)});
  });

  <div>
    {React.string(
       displaySeconds(int_of_float((now -. startTimestamp) /. 1000.)),
     )}
    {React.string("/")}
    {React.string(
       displaySeconds(int_of_float(roomTrack.durationMs /. 1000.)),
     )}
  </div>;
};