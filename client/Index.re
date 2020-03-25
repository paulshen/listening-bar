open Webapi.Dom;
open Belt.Option;

{
  open Css;
  global(
    "body",
    [
      backgroundColor(hex("3E312B")),
      color(hex("f0f0f0")),
      fontFamily(`custom("Barlow Semi Condensed")),
      margin(zero),
    ],
  );
  global("a", [color(hex("f0f0f0")), textDecoration(none)]);
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