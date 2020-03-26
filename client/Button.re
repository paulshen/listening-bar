module Styles = {
  open Css;
  let button =
    style([
      backgroundColor(hex("FDFEC3C0")),
      // border(px(1), solid, hex("f0f0f0")),
      borderWidth(zero),
      color(hex("6c564b")),
      fontSize(px(16)),
      fontWeight(`num(500)),
      padding2(~v=px(6), ~h=px(12)),
      borderRadius(px(4)),
      textTransform(uppercase),
      transition(~duration=200, "background-color"),
      hover([backgroundColor(hex("FDFEC3"))]),
    ]);
};

[@react.component]
let make =
    (~onClick=?, ~onMouseEnter=?, ~onMouseLeave=?, ~domRef=?, ~children) => {
  <button
    ?onClick ?onMouseEnter ?onMouseLeave className=Styles.button ref=?domRef>
    children
  </button>;
};