open Belt;
open Express;

[@bs.module]
external bodyParser: {. [@bs.meth] "json": unit => Middleware.t} =
  "body-parser";
[@bs.module] external cors: unit => Middleware.t = "cors";
let app = express();

App.disable(app, ~name="x-powered-by");
App.use(app, bodyParser##json());
App.use(app, cors());

let promiseMiddleware = middleware => {
  PromiseMiddleware.from((next, req, res) =>
    Promise.Js.toBsPromise(middleware(next, req, res))
  );
};
let setProperty = (req, property, value, res) => {
  let reqData = Request.asJsonObject(req);
  Js.Dict.set(reqData, property, value);
  res;
};
let getProperty = (req, property) => {
  let reqData = Request.asJsonObject(req);
  Js.Dict.get(reqData, property);
};

App.get(app, ~path="/hello") @@
promiseMiddleware((next, req, res) =>
  res |> Response.sendString("Hello World!") |> Promise.Js.resolved
);

App.post(app, ~path="/login") @@
promiseMiddleware((next, req, res) => {
  let bodyJson = Request.bodyJSON(req) |> Option.getExn;
  let code =
    bodyJson
    ->Js.Json.decodeObject
    ->Option.getExn
    ->Js.Dict.unsafeGet("code")
    ->Js.Json.decodeString
    ->Option.getExn;
  let%Repromise (sessionId, accessToken) = Spotify.getToken(code);
  let responseJson =
    Js.Json.object_(
      Js.Dict.fromArray([|
        ("sessionId", Js.Json.string(sessionId)),
        ("accessToken", Js.Json.string(accessToken)),
      |]),
    );
  res |> Response.sendJson(responseJson) |> Promise.resolved;
});

App.getWithMany(app, ~path="/user") @@
[|
  Middleware.from((next, req, res) => {
    switch (
      Request.get("Authorization", req)->Option.flatMap(Persist.getSession)
    ) {
    | Some(session) =>
      setProperty(req, "session", Obj.magic(session), res) |> ignore;
      next(Next.middleware, res);
    | None => Response.sendStatus(Response.StatusCode.Unauthorized, res)
    }
  }),
  promiseMiddleware((next, req, res) => {
    let userId: option(string) =
      Option.map(getProperty(req, "session"), json =>
        Obj.magic(json)##userId
      );
    let responseJson =
      Js.Json.object_(
        Js.Dict.fromArray([|
          (
            "userId",
            switch (userId) {
            | Some(userId) => Js.Json.string(userId)
            | None => Js.Json.null
            },
          ),
        |]),
      );
    res |> Response.sendJson(responseJson) |> Promise.resolved;
  }),
|];

let onListen = e =>
  switch (e) {
  | exception (Js.Exn.Error(e)) =>
    Js.log(e);
    Node.Process.exit(1);
  | _ => Js.log @@ "Listening at http://127.0.0.1:3000"
  };

let server = App.listen(app, ~port=3000, ~onListen, ());