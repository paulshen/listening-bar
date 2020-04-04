[@bs.scope ("process", "env")] [@bs.val]
external nodeEnv: option(string) = "NODE_ENV";

let isProduction = nodeEnv === Some("production");

let serverUrl =
  isProduction ? "https://api.listening.bar" : "http://localhost:3030";
let clientUrl =
  isProduction ? "https://listening.bar" : "http://localhost:8000";

let foreverRoomIds = [|"kid-a", "kind-of-blue", "blue-train"|];