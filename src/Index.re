open Webapi.Dom;
open Belt.Option;

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