[@bs.scope ("process", "env")] [@bs.val]
external nodeEnv: option(string) = "NODE_ENV";

let serverUrl =
  nodeEnv === Some("production")
    ? "https://api.listening.bar" : "http://localhost:3030";
let clientUrl =
  nodeEnv === Some("production") ? "https://listening.bar" : "http://localhost:8000";