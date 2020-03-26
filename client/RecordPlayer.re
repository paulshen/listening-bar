module Styles = {
  open Css;
  let root =
    style([
      backgroundColor(hex("ab9270")),
      borderRadius(px(4)),
      width(px(320)),
      height(px(208)),
      position(relative),
    ]);
  let turntableContainer =
    style([
      width(px(192)),
      height(px(192)),
      position(absolute),
      top(px(8)),
      left(px(8)),
    ]);
  let turntable =
    style([
      unsafe(
        "background",
        "#1e1e1e linear-gradient(135deg, rgba(255,255,255,0) 40%, rgba(255,255,255,0.15) 50%, rgba(255,255,255,0) 60%);",
      ),
      borderRadius(pct(50.)),
      position(absolute),
      top(zero),
      left(zero),
      width(pct(100.)),
      height(pct(100.)),
    ]);
  let recordCenter =
    style([
      backgroundColor(hex("e0e0e0")),
      borderRadius(pct(50.)),
      width(px(48)),
      height(px(48)),
      position(absolute),
      top(pct(50.)),
      left(pct(50.)),
      marginLeft(px(-24)),
      marginTop(px(-24)),
    ]);
  let recordCenterRectangle =
    style([
      backgroundColor(hex("606060")),
      width(px(20)),
      height(px(3)),
      position(absolute),
      left(pct(50.)),
      marginLeft(px(-10)),
      top(pct(50.)),
      marginTop(px(-14)),
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
      backgroundColor(hex("82796d")),
      width(px(6)),
      height(px(176)),
      borderBottomLeftRadius(px(3)),
      borderBottomRightRadius(px(3)),
      borderTopRightRadius(px(2)),
    ]);
  let toneArmShortSegment =
    style([
      backgroundColor(hex("82796d")),
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
  let userId =
    style([
      fontSize(px(14)),
      position(absolute),
      right(px(16)),
      top(px(12)),
      opacity(0.5),
      transition(~duration=200, "opacity"),
      hover([opacity(1.)]),
    ]);
};

let recordStopDegree = (-30.);
let recordPlayStartDegree = (-37.);
let recordPlayEndDegree = (-57.);

module SpringHook =
  Spring.MakeSpring({
    type t = float;
    type interpolate = float => string;
  });

[@bs.module "react-spring"]
external interpolate2: (('a, 'b), ('a, 'b) => string) => string =
  "interpolate";

[@react.component]
let make =
    (
      ~userId: option(string),
      ~startTimestamp: option(float),
      ~totalDuration: option(float),
      ~className,
    ) => {
  let (_, forceUpdate) = React.useState(() => 1);
  let playProgress =
    switch (startTimestamp, totalDuration) {
    | (Some(startTimestamp), Some(totalDuration)) =>
      let now = Js.Date.now();
      Some((now -. startTimestamp) /. totalDuration);
    | _ => None
    };

  let (progressSpring, setProgressSpring) =
    SpringHook.use(
      ~config=Spring.config(~mass=1., ~tension=80., ~friction=20.),
      recordStopDegree,
    );
  let (playJitter, setPlayJitter) =
    SpringHook.use(
      ~config=Spring.config(~mass=1., ~tension=80., ~friction=50.),
      0.,
    );
  let (turntableJitter, setTurntableJitter) =
    SpringHook.use(
      ~config=Spring.config(~mass=20., ~tension=60., ~friction=80.),
      0.,
    );

  let isPlaying = Belt.Option.isSome(playProgress);
  React.useEffect1(
    () =>
      if (isPlaying) {
        let x = ref(true);
        let jitterInterval =
          Js.Global.setInterval(
            () => {
              setPlayJitter(x^ ? 1 : 0);
              setTurntableJitter(x^ ? 1 : 0);
              x := ! x^;
            },
            800,
          );
        let playInterval =
          Js.Global.setInterval(() => {forceUpdate(x => x + 1)}, 10000);
        Some(
          () => {
            Js.Global.clearInterval(jitterInterval);
            Js.Global.clearInterval(playInterval);
          },
        );
      } else {
        None;
      },
    [|isPlaying|],
  );

  React.useEffect1(
    () => {
      let armDegree =
        switch (playProgress) {
        | Some(playProgress) =>
          recordPlayStartDegree
          +. (recordPlayEndDegree -. recordPlayStartDegree)
          *. playProgress
        | None => recordStopDegree
        };

      setProgressSpring(armDegree);
      None;
    },
    [|playProgress|],
  );

  let recordCenterRef = React.useRef(Js.Nullable.null);
  let rotateDegreeRef = React.useRef(0.);
  let lastRotateUpdateRef = React.useRef(0.);
  let rotateVelocityRef = React.useRef(0.);
  let rotateTargetVelocityRef = React.useRef(0.);
  let rafRotationRef = React.useRef(None);
  let rec updateRotation = () => {
    let velocity = React.Ref.current(rotateVelocityRef);
    let now = Js.Date.now();
    let frameSeconds =
      (now -. React.Ref.current(lastRotateUpdateRef)) /. 1000.;
    if (velocity > 0.) {
      let degree =
        React.Ref.current(rotateDegreeRef) +. velocity *. frameSeconds;
      recordCenterRef
      ->React.Ref.current
      ->Js.Nullable.toOption
      ->Belt.Option.getExn
      ->Webapi.Dom.Element.unsafeAsHtmlElement
      ->Webapi.Dom.HtmlElement.style
      |> Webapi.Dom.CssStyleDeclaration.setProperty(
           "transform",
           "rotate(" ++ Js.Float.toString(degree) ++ "deg)",
           "",
         );
      React.Ref.setCurrent(rotateDegreeRef, degree);
    };
    let targetVelocity = React.Ref.current(rotateTargetVelocityRef);
    let velocity =
      if (velocity < targetVelocity) {
        Js.Math.min_float(
          velocity +. 100. *. Js.Math.min_float(frameSeconds, 0.3),
          targetVelocity,
        );
      } else if (velocity > targetVelocity) {
        Js.Math.max_float(
          velocity -. 40. *. Js.Math.min_float(frameSeconds, 0.3),
          targetVelocity,
        );
      } else {
        velocity;
      };
    React.Ref.setCurrent(lastRotateUpdateRef, now);
    React.Ref.setCurrent(rotateVelocityRef, velocity);
    if (velocity > 0.) {
      React.Ref.setCurrent(
        rafRotationRef,
        Some(Webapi.requestCancellableAnimationFrame(_ => updateRotation())),
      );
    } else {
      React.Ref.setCurrent(rafRotationRef, None);
    };
  };
  React.useEffect1(
    () => {
      React.Ref.setCurrent(rotateTargetVelocityRef, isPlaying ? 200. : 0.);
      React.Ref.setCurrent(
        rafRotationRef,
        Some(Webapi.requestCancellableAnimationFrame(_ => updateRotation())),
      );
      Some(
        () => {
          switch (React.Ref.current(rafRotationRef)) {
          | Some(rafRotation) =>
            Webapi.cancelAnimationFrame(rafRotation);
            React.Ref.setCurrent(rafRotationRef, None);
          | None => ()
          }
        },
      );
    },
    [|isPlaying|],
  );

  <div className={Cn.make([Styles.root, className])}>
    <div className=Styles.turntableContainer>
      <Spring.Div
        className=Styles.turntable
        style={ReactDOMRe.Style.make(
          ~transform=
            turntableJitter->SpringHook.interpolate(turntableJitter => {
              let deg = turntableJitter *. 20.;
              {j|rotate($(deg)deg)|j};
            }),
          (),
        )}
      />
      <div
        className=Styles.recordCenter
        ref={ReactDOMRe.Ref.domRef(recordCenterRef)}>
        <div className=Styles.recordCenterRectangle />
      </div>
      <div className=Styles.turntableRod />
    </div>
    <div className=Styles.toneArmHolder />
    <Spring.Div
      className=Styles.toneArm
      style={ReactDOMRe.Style.make(
        ~transform=
          interpolate2(
            (progressSpring, playJitter),
            (progressSpring, playJitter) => {
              let deg = progressSpring +. (playJitter -. 0.5) *. 2.;
              {j|rotate($(deg)deg)|j};
            },
          ),
        (),
      )}>
      <div className=Styles.toneArmShortSegment>
        <div className=Styles.toneArmCartridge />
      </div>
      <div className=Styles.toneArmLongSegment />
    </Spring.Div>
    {switch (userId) {
     | Some(userId) =>
       <div className=Styles.userId> {React.string(userId)} </div>
     | None => React.null
     }}
  </div>;
};