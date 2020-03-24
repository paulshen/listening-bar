let serverUrl =
  Js.Dict.get(Node.Process.process##env, "NODE_ENV") === Some("production")
    ? "http://listening-room.bypaulshen.com" : "http://localhost:3030";