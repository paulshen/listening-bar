open Express;

let app = express();

App.disable(app, ~name="x-powered-by");

App.get(app, ~path="/hello") @@
PromiseMiddleware.from((_req, _next, res) =>
  res
  |> Response.sendString("Hello World!")
  |> Promise.Js.resolved
  |> Promise.Js.toBsPromise
);

let onListen = e =>
  switch (e) {
  | exception (Js.Exn.Error(e)) =>
    Js.log(e);
    Node.Process.exit(1);
  | _ => Js.log @@ "Listening at http://127.0.0.1:3000"
  };

let server = App.listen(app, ~port=3000, ~onListen, ());