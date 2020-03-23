[@react.component]
let make = () => {
  let user = UserStore.useUser();
  switch (user) {
  | Some(user) => <div> {React.string(user.id)} </div>
  | None => React.null
  };
};