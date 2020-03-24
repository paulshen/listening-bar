module Styles = {
  open Css;
  let root =
    style([
      backgroundColor(hex("CFC1AE")),
      borderRadius(px(4)),
      width(px(320)),
      height(px(208)),
      position(relative),
    ]);
  let turntable =
    style([
      unsafe(
        "background",
        "#1e1e1e linear-gradient(150deg, rgba(255,255,255,0) 35%, rgba(255,255,255,0.25) 50%, rgba(255,255,255,0) 65%);",
      ),
      width(px(192)),
      height(px(192)),
      borderRadius(pct(50.)),
      position(absolute),
      top(px(8)),
      left(px(8)),
    ]);
  let recordCenter =
    style([
      backgroundColor(hex("C2B19B")),
      borderRadius(pct(50.)),
      width(px(48)),
      height(px(48)),
      position(absolute),
      top(pct(50.)),
      left(pct(50.)),
      marginLeft(px(-24)),
      marginTop(px(-24)),
    ]);
  let turntableRod =
    style([
      backgroundColor(hex("1E1E1E")),
      position(absolute),
      borderRadius(pct(50.)),
      height(px(4)),
      width(px(4)),
      top(pct(50.)),
      left(pct(50.)),
      marginLeft(px(-2)),
      marginTop(px(-2)),
    ]);
  let toneArmHolder =
    style([
      backgroundColor(hex("C2B19B")),
      position(absolute),
      borderRadius(pct(50.)),
      height(px(32)),
      width(px(32)),
      bottom(px(24)),
      right(px(24)),
    ]);
  let toneArm =
    style([
      position(absolute),
      right(px(36)),
      bottom(zero),
      transformOrigin(pct(50.), px(238 - 24 - 16)),
    ]);
  let toneArmLongSegment =
    style([
      backgroundColor(hex("958D82")),
      width(px(6)),
      height(px(176)),
      borderBottomLeftRadius(px(3)),
      borderBottomRightRadius(px(3)),
      borderTopRightRadius(px(2)),
    ]);
  let toneArmShortSegment =
    style([
      backgroundColor(hex("958D82")),
      width(px(6)),
      height(px(64)),
      marginBottom(px(-2)),
      transformOrigin(zero, pct(100.)),
      transforms([rotate(deg(-15.))]),
    ]);
  let toneArmCartridge =
    style([
      backgroundColor(hex("b5aDa2")),
      width(px(10)),
      height(px(32)),
      position(relative),
      left(px(-2)),
    ]);
};

let recordStopDegree = 30.;
let recordPlayStartDegree = 37.;
let recordPlayEndDegree = 57.;

[@react.component]
let make = (~playProgress) => {
  <div className=Styles.root>
    <div className=Styles.turntable>
      <div className=Styles.recordCenter />
      <div className=Styles.turntableRod />
    </div>
    <div className=Styles.toneArmHolder />
    <div
      className=Styles.toneArm
      style={ReactDOMRe.Style.make(
        ~transform=
          "rotate("
          ++ Js.Float.toString(
               -. (
                 switch (playProgress) {
                 | Some(playProgress) =>
                   recordPlayStartDegree
                   +. (recordPlayEndDegree -. recordPlayStartDegree)
                   *. playProgress
                 | None => recordStopDegree
                 }
               ),
             )
          ++ "deg)",
        (),
      )}>
      <div className=Styles.toneArmShortSegment>
        <div className=Styles.toneArmCartridge />
      </div>
      <div className=Styles.toneArmLongSegment />
    </div>
  </div>;
};