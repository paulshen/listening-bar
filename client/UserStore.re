type user = {
  id: string,
  accessToken: string,
};

type state = {
  sessionId: option(string),
  user: option(user),
};
type action =
  | Login(string)
  | UpdateUser(user);

let sessionId = Dom.Storage.(localStorage |> getItem("sessionId"));

let api =
  Restorative.createStore({sessionId, user: None}, (state, action) => {
    switch (action) {
    | Login(sessionId) => {sessionId: Some(sessionId), user: None}
    | UpdateUser(user) => {...state, user: Some(user)}
    }
  });

let getSessionId = () => api.getState().sessionId;
let useUser = () => api.useStoreWithSelector(state => state.user, ());
let getUser = () => api.getState().user;

let login = sessionId => api.dispatch(Login(sessionId));
let updateUser = (user: user) => {
  api.dispatch(UpdateUser(user));
};