[@bs.scope ("process", "env")] [@bs.val]
external nodeEnv: option(string) = "NODE_ENV";

let serverUrl =
  nodeEnv === Some("production")
    ? "http://listening-room.bypaulshen.com" : "http://localhost:3030";
let clientUrl =
  nodeEnv === Some("production") ? serverUrl : "http://localhost:8000";