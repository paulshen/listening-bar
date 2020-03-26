open Webapi.Dom;
open Belt.Option;

{
  open Css;
  global(
    "body, button, input",
    [fontFamily(`custom("Barlow Semi Condensed, sans-serif"))],
  );
  global("body", [color(hex("f0f0f0")), margin(zero)]);
  global(
    "a",
    [
      color(hex("f0f0f0")),
      textDecoration(none),
      hover([textDecoration(underline)]),
    ],
  );
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