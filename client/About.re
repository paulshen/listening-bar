module Styles = {
  open Css;
  let root =
    style([
      display(flexBox),
      fontSize(px(14)),
      alignItems(center),
      justifyContent(center),
      padding2(~v=px(64), ~h=px(16)),
      media(
        "(max-width: 720px)",
        [
          display(block),
          width(px(320)),
          marginLeft(auto),
          marginRight(auto),
        ],
      ),
      selector("& p", [marginTop(zero), marginBottom(px(24))]),
    ]);
  let pane = style([width(px(320))]);
  let paneSpacer =
    style([
      width(px(64)),
      media("(min-width: 1000px)", [width(px(128))]),
    ]);
  let drawing = style([width(px(320)), height(px(320))]);
  let logo =
    style([
      textIndent(px(-9999)),
      width(px(245)),
      height(px(60)),
      backgroundImage(url("/assets/logo.png")),
      backgroundSize(cover),
      marginTop(zero),
      marginBottom(px(32)),
    ]);

  let roomForm = style([marginBottom(px(24))]);
  let roomFormRow = style([display(flexBox)]);
  let returnToRoom =
    style([
      color(hex("FDFEC3c0")),
      display(inlineBlock),
      fontSize(px(12)),
      textTransform(uppercase),
      marginTop(px(4)),
    ]);
  let roomInput =
    style([
      backgroundColor(transparent),
      borderWidth(zero),
      borderBottom(px(1), solid, rgba(255, 255, 255, 0.5)),
      boxSizing(borderBox),
      color(hex("f0f0f0")),
      padding(zero),
      height(px(29)),
      flexGrow(1.),
      fontSize(px(14)),
      marginRight(px(16)),
      outlineStyle(none),
      selector(
        "&::-webkit-input-placeholder",
        [color(rgba(255, 255, 255, 0.6))],
      ),
      transition(~duration=200, "border-bottom-color"),
      focus([borderBottomColor(hex("FDFEC3c0"))]),
    ]);
};

[@react.component]
let make = (~roomId=?, ()) => {
  let roomInputRef = React.useRef(Js.Nullable.null);
  let onRoomSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    let roomId =
      roomInputRef
      ->React.Ref.current
      ->Js.Nullable.toOption
      ->Belt.Option.getExn
      ->Webapi.Dom.Element.unsafeAsHtmlElement
      ->Webapi.Dom.HtmlElement.value;
    ReasonReactRouter.push("/" ++ roomId);
  };

  <div className=Styles.root>
    <div className=Styles.pane>
      <img src="/assets/drawing.png" className=Styles.drawing />
    </div>
    <div className=Styles.paneSpacer />
    <div className=Styles.pane>
      <h1 className=Styles.logo> {React.string("listening.bar")} </h1>
      <p>
        {React.string(
           "This is a place for shared listening of Spotify albums. Inspired by modern listening bars, where people gather not to gossip but to listen and appreciate sounds together.",
         )}
      </p>
      <p>
        {React.string(
           "Create or join a room by ID. Login with your Spotify Premium account. Sync to listen with others.",
         )}
      </p>
      <form onSubmit=onRoomSubmit className=Styles.roomForm>
        <div className=Styles.roomFormRow>
          <input
            type_="text"
            placeholder="Room ID"
            className=Styles.roomInput
            ref={ReactDOMRe.Ref.domRef(roomInputRef)}
          />
          <Button> {React.string("Enter Room")} </Button>
        </div>
        {switch (roomId) {
         | Some(roomId) =>
           <a
             href={j|/$roomId|j}
             onClick={e => {
               ReactEvent.Mouse.preventDefault(e);
               ReasonReactRouter.replace({j|/$roomId|j});
             }}
             className=Styles.returnToRoom>
             {React.string("Return to room " ++ roomId)}
           </a>
         | None => React.null
         }}
      </form>
      <p>
        {React.string(
           {|To change a room's album, play a song in your Spotify and click "Put On Album". This will play your song's album starting from the beginning of your track to the end of the album. Others can join your room with URL https://listening.bar/roomId|},
         )}
      </p>
      <p>
        {React.string("Something not working or feature request?")}
        <br />
        {React.string("Tweet")}
        <a href="https://twitter.com/_paulshen" target="_blank">
          {React.string("@_paulshen")}
        </a>
      </p>
    </div>
  </div>;
};