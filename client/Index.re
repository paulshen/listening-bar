open Webapi.Dom;
open Belt.Option;

{
  open Css;
  global(
    "body",
    [
      backgroundColor(hex("3c3027")),
      color(hex("f0f0f0")),
      fontFamily(`custom("Noto Sans")),
    ],
  );
  global("a", [color(hex("f0f0f0"))]);
};

let root =
  document
  |> Document.asHtmlDocument
  |> getExn
  |> HtmlDocument.createElement("div");

document
|> Document.asHtmlDocument
|> getExn
|> HtmlDocument.body
|> getExn
|> Element.appendChild(root);

ReactDOMRe.render(<App />, root);