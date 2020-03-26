module Styles = {
  open Css;
  let button =
    style([
      backgroundColor(hex("FDFEC3c0")),
      // border(px(1), solid, hex("f0f0f0")),
      borderWidth(zero),
      color(hex("6c564b")),
      fontSize(px(14)),
      fontWeight(`num(500)),
      padding2(~v=px(6), ~h=px(12)),
      borderRadius(px(2)),
      textTransform(uppercase),
      transition(~duration=200, "background-color"),
      cursor(pointer),
      hover([backgroundColor(hex("FDFEC3"))]),
    ]);
};

[@react.component]
let make =
    (
      ~onClick=?,
      ~onMouseEnter=?,
      ~onMouseLeave=?,
      ~domRef=?,
      ~className=?,
      ~children,
    ) => {
  <button
    ?onClick
    ?onMouseEnter
    ?onMouseLeave
    className={Cn.make([Styles.button, Cn.unpack(className)])}
    ref=?domRef>
    children
  </button>;
};