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

App.get(app, ~path="/hello") @@
promiseMiddleware((next, req, res) =>
  res |> Response.sendString("Hello World!") |> Promise.Js.resolved
);

App.post(app, ~path="/login") @@
promiseMiddleware((next, req, res) => {
  let bodyJson = Request.bodyJSON(req) |> Option.getExn;
  let token =
    bodyJson
    ->Js.Json.decodeObject
    ->Option.getExn
    ->Js.Dict.unsafeGet("token")
    ->Js.Json.decodeString
    ->Option.getExn;
  let responseJson =
    Js.Json.object_(
      Js.Dict.fromArray([|("passToken", Js.Json.string(token))|]),
    );
  Promise.resolved(Response.sendJson(responseJson, res));
});

let onListen = e =>
  switch (e) {
  | exception (Js.Exn.Error(e)) =>
    Js.log(e);
    Node.Process.exit(1);
  | _ => Js.log @@ "Listening at http://127.0.0.1:3000"
  };

let server = App.listen(app, ~port=3000, ~onListen, ());